"""One producer, zero wheels, bounded lab soak. No reconnect resumes a target."""
import argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime, timezone
import csv
import hashlib
import json
import math
import os
from pathlib import Path
import random
import signal
import socket
import tempfile
import threading
import time
import urllib.error
import urllib.parse
import urllib.request

SCENARIOS = ('idle', 'oled_off', 'reactive', 'sharingan', 'concurrency',
             'servo', 'laser', 'servo_laser', 'slow_client', 'producer_expiry')


class SafetyError(RuntimeError):
    pass


def zero_wheels(body):
    if any(type(body.get(k)) not in (int, float) or body[k] != 0 for k in ('left', 'right')):
        raise SafetyError('Every actuator command MUST have left=0 and right=0')
    for key in ('arm_left', 'arm_right'):
        if type(body.get(key)) not in (int, float) or not math.isfinite(body[key]) or not 0 <= body[key] <= 180:
            raise SafetyError('Invalid arm target')
    if type(body.get('laser')) is not bool:
        raise SafetyError('Invalid laser target')


def verify_identity(s, mac, version, build):
    h = s.get('hardware', {})
    if (s.get('sta_mac', '').lower() != mac.lower() or s.get('firmware_version') != version or
            s.get('build_time_utc') != build or h.get('chip_model') != 'ESP32-S3' or
            h.get('flash_size_bytes') != 16777216 or h.get('psram_size_bytes') != 8388608 or
            s.get('access_mode') != 'open_lan'):
        raise SafetyError('Device/build identity mismatch')


class DeviceLock:
    """OS-held lock shared across output paths; process death releases it."""
    def __init__(self, url):
        key = hashlib.sha256(url.rstrip('/').encode()).hexdigest()[:20]
        self.path = Path(tempfile.gettempdir()) / ('robot-soak-' + key + '.lock')

    def __enter__(self):
        self.file = self.path.open('a+b')
        self.file.seek(0)
        self.file.write(b'0')
        self.file.flush()
        self.file.seek(0)
        try:
            if os.name == 'nt':
                import msvcrt
                msvcrt.locking(self.file.fileno(), msvcrt.LK_NBLCK, 1)
            else:
                import fcntl
                fcntl.flock(self.file, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except OSError:
            self.file.close()
            raise SafetyError('Another runner holds this device lock')
        return self

    def __exit__(self, *_):
        self.file.close()


class Runner:
    def __init__(self, args):
        self.args = args
        self.out = args.output
        self.out.mkdir(parents=True, exist_ok=False)
        self.log = (self.out / 'events.jsonl').open('x', encoding='utf-8')
        self.csvfile = (self.out / 'requests.csv').open('x', newline='', encoding='utf-8')
        self.csv = csv.writer(self.csvfile)
        self.csv.writerow(('elapsed_s', 'scenario', 'path', 'ok', 'latency_ms', 'error'))
        self.mu = threading.Lock()
        self.cancel = threading.Event()
        self.start = time.monotonic()
        self.deadline = self.start + args.hours * 3600
        self.session = 0
        self.sequence = 0
        self.scenario = 'setup'
        self.counts = Counter()
        self.hist = Counter()
        self.command_hist = Counter()
        self.scenario_seconds = Counter()
        self.max_latency = 0
        self.max_gap = 0
        self.last_ack = None
        self.last_uptime = None
        self.first_disconnects = None
        self.latest_status = None
        self.last_checkpoint = 0
        self.pool = ThreadPoolExecutor(max_workers=2)
        self.pending = []
        self.rng = random.Random(args.seed)
        self.base = None
        self.slow_socket = None
        self.initial_mode = None
        self.initial_eyes = None
        self.initial_arm_motion = None
        self.owns_mode = False
        self.event('configuration', config={**vars(args), 'output': str(args.output)}, pid=os.getpid())

    def event(self, kind, **fields):
        with self.mu:
            self.log.write(json.dumps(dict(event=kind, utc=datetime.now(timezone.utc).isoformat(),
                                          elapsed_s=time.monotonic()-self.start,
                                          scenario=self.scenario, **fields), allow_nan=False) + '\n')
            self.log.flush()

    def request(self, path, body=None, timeout=0.3):
        # The final transmission boundary repeats the wheel guard.
        if path == '/command':
            zero_wheels(body)
            self.event('command_attempt', body=body)
        begin = time.monotonic()
        ok, error = False, ''
        try:
            req = urllib.request.Request(self.args.url.rstrip('/') + '/api/v1' + path,
                                         data=None if body is None else json.dumps(body).encode(),
                                         headers={'Content-Type': 'application/json', 'Connection': 'close'})
            opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))
            with opener.open(req, timeout=timeout) as r:
                result = json.load(r)
            ok = True
            return result
        except Exception as e:
            error = str(e)
            raise
        finally:
            ms = (time.monotonic()-begin)*1000
            with self.mu:
                self.counts['requests'] += 1
                self.counts['request_failures'] += not ok
                self.hist[math.ceil(ms)] += 1
                if path == '/command':
                    self.command_hist[math.ceil(ms)] += 1
                    self.counts['commands'] += 1
                    self.counts['command_failures'] += not ok
                self.max_latency = max(self.max_latency, ms)
                self.csv.writerow((round(begin-self.start, 6), self.scenario, path, ok, round(ms, 3), error))
                self.csvfile.flush()
            if not ok:
                self.event('request_failure', path=path, latency_ms=ms, error=error)

    def status(self, timeout=0.3):
        expected_session=self.session
        s = self.request('/status', timeout=timeout)
        verify_identity(s, self.args.mac, self.args.version, self.args.build)
        if s.get('maintenance'):
            raise SafetyError('Device is in maintenance')
        if s.get('owner') != 'none' and (not expected_session or s.get('session') != expected_session):
            raise SafetyError('Another controller owns the robot')
        out = s['commanded_output']
        if out['left'] != 0 or out['right'] != 0 or any(s['pwm'][k] != 0 for k in
                ('left1_duty', 'left2_duty', 'right1_duty', 'right2_duty')):
            raise SafetyError('Observed nonzero wheel output')
        uptime = s['uptime_ms']
        if self.last_uptime is not None and uptime < self.last_uptime and self.last_uptime < 4294900000:
            self.counts['unexpected_reboots'] += 1
            raise SafetyError('Unexpected reboot')
        self.last_uptime = uptime
        disconnects=s.get('wifi_disconnect_count',0)
        if self.first_disconnects is None:
            self.first_disconnects=disconnects
        self.counts['wifi_disconnects']=max(0,disconnects-self.first_disconnects)
        self.latest_status = s
        self.event('status', status=s)
        return s

    def check_cancel(self):
        if self.cancel.is_set() or (self.out / 'stop.flag').exists():
            raise InterruptedError('Cancelled')

    def wait(self, seconds):
        end = min(time.monotonic() + seconds, self.deadline)
        while time.monotonic() < end:
            self.check_cancel()
            self.cancel.wait(min(.1, max(0, end-time.monotonic())))

    def command(self, left, right, laser):
        self.check_cancel()
        # A delayed producer exits instead of refreshing a stale target.
        if self.last_ack is not None:
            gap = time.monotonic() - self.last_ack
            self.max_gap = max(self.max_gap, gap*1000)
            if gap >= .3:
                raise TimeoutError('Producer freshness lost before transmission')
        body = dict(session=self.session, sequence=self.sequence+1, left=0, right=0,
                    arm_left=left, arm_right=right, laser=laser)
        zero_wheels(body)
        begin = time.monotonic()
        self.request('/command', body)
        if self.last_ack is not None:
            self.max_gap = max(self.max_gap, (time.monotonic()-self.last_ack)*1000)
        self.sequence += 1
        self.last_ack = time.monotonic()
        if self.last_ack - begin >= .3:
            raise TimeoutError('Command acknowledgement exceeded producer freshness')

    def release(self):
        session, self.session = self.session, 0
        self.last_ack = None
        if session:
            try:
                self.request('/control/release', {'session': session}, timeout=1)
            except Exception as e:
                self.event('release_failed', error=str(e))
        # Never global-stop a different owner's newer session.

    def drain(self):
        pending, self.pending = self.pending, []
        failed = False
        for f in pending:
            try:
                f.result()
            except SafetyError:
                raise
            except Exception:
                failed = True
        if failed:
            raise TimeoutError('Concurrent traffic failed; abandoning current target')

    def cosmetic(self, name):
        e = dict(self.initial_eyes)
        e.update(enabled=name != 'oled_off', automatic=name != 'sharingan',
                 expression='sharingan', category='sharingan' if name in ('sharingan', 'concurrency') else 'all',
                 play_mode='manual' if name == 'sharingan' else 'showcase' if name == 'concurrency' else 'reactive')
        self.request('/eyes', e, timeout=2)

    def run_scenario(self, name, duration):
        self.scenario = name
        self.drain()
        s = self.status(timeout=2)
        if s['owner'] != 'none' or s['commanded_output']['laser']:
            raise SafetyError('Scenario requires released, laser-off outputs')
        self.event('scenario_start', duration_s=duration, status=s)
        self.cosmetic(name)
        active = name in ('servo', 'laser', 'servo_laser', 'concurrency', 'producer_expiry')
        if active:
            self.request('/arms/settings', dict(speed_dps=5, acceleration_dps2=20), timeout=2)
            self.status(timeout=2)  # Owner/maintenance check immediately before acquire.
            result = self.request('/control/acquire', {'owner': 'program'}, timeout=0.3)
            self.session = result['session']
            if result.get('timeout_ms') != 500:
                raise SafetyError('Firmware expiry must remain 500 ms')
            self.sequence = 0
            self.last_ack = time.monotonic()
        begin = time.monotonic()
        end = min(begin + duration, self.deadline)
        next_command, next_status, next_load = begin, begin, begin
        expiry_at = None
        if name == 'slow_client':
            u = urllib.parse.urlparse(self.args.url)
            self.slow_socket = socket.create_connection((u.hostname, u.port or 80), timeout=2)
            self.slow_socket.sendall(b'GET /api/v1/status HTTP/1.1\r\nHost: robot\r\n')
            # Closing the test socket is independent of a possibly blocked status read.
            closer=threading.Timer(.25, self.slow_socket.close)
            closer.daemon=True
            closer.start()
        try:
            while time.monotonic() < end:
                self.check_cancel()
                now = time.monotonic()
                if self.pending and all(f.done() for f in self.pending):
                    self.drain()
                if self.slow_socket and now-begin >= .25:
                    self.slow_socket.close()
                    self.slow_socket = None
                if active and now >= next_command and expiry_at is None:
                    t = now-begin
                    # At a boundary pose (e.g. zero), only move inward by up to two degrees.
                    arms = [min(178, max(2, a)) + 1.0*math.sin(t*.4) for a in self.base]
                    if name == 'laser' or name == 'producer_expiry':
                        arms = self.base
                    self.command(*arms, name in ('laser', 'servo_laser', 'producer_expiry') and int(t/2)%2 == 0)
                    next_command = time.monotonic() + .1
                    if name == 'producer_expiry':
                        expiry_at = time.monotonic()
                if name == 'concurrency' and now >= next_load:
                    if not self.pending:
                        self.pending = [self.pool.submit(self.request, path) for path in ('/status', '/eyes/catalog')]
                    next_load = now + 3
                # Status is kept off the active producer path: no background actuator sender exists.
                if now >= next_status and (not active or expiry_at is not None):
                    if expiry_at is not None and now-expiry_at < .7:
                        self.wait(.05)
                        continue
                    observed = self.status(timeout=2 if not active else .3)
                    if expiry_at is not None:
                        if observed['session'] or observed['commanded_output']['laser']:
                            raise SafetyError('Producer expiry did not stop/invalidate session')
                        self.event('expiry_verified', status=observed)
                        self.session = 0
                        break
                    next_status = now+1
                if active and expiry_at is None and now >= next_status:
                    if not self.pending:
                        self.pending = [self.pool.submit(self.status)]
                    next_status = now+1
                self.checkpoint('running')
                self.wait(.01 if active else .1)
        finally:
            if self.slow_socket:
                self.slow_socket.close()
                self.slow_socket = None
            self.release()
            self.drain()
            self.scenario_seconds[name] += time.monotonic()-begin
            self.event('scenario_end')

    def checkpoint(self, state, force=False, error=None):
        if not force and time.monotonic()-self.last_checkpoint < 5:
            return
        self.last_checkpoint = time.monotonic()
        with self.mu:
            total = sum(self.hist.values())
            def pct(q, histogram=None):
                histogram=self.hist if histogram is None else histogram
                total=sum(histogram.values())
                target, accum = math.ceil(q*total), 0
                for ms, n in sorted(histogram.items()):
                    accum += n
                    if accum >= target:
                        return ms
                return None
            summary = dict(state=state, pid=os.getpid(), url=self.args.url, scenario=self.scenario,
                           updated_utc=datetime.now(timezone.utc).isoformat(),
                           elapsed_s=time.monotonic()-self.start, planned_s=self.args.hours*3600,
                           counts=dict(self.counts), p95_ms=pct(.95), p99_ms=pct(.99),
                           command_p95_ms=pct(.95,self.command_hist), command_p99_ms=pct(.99,self.command_hist),
                           worst_latency_ms=self.max_latency, worst_ack_gap_ms=self.max_gap,
                           scenario_seconds=dict(self.scenario_seconds), error=error,
                           latest_status=self.latest_status)
            temp = self.out / 'checkpoint.tmp'
            temp.write_text(json.dumps(summary, indent=2), encoding='utf-8')
            temp.replace(self.out / 'checkpoint.json')

    def run(self):
        state, error = 'failed', None
        try:
            s = self.status(timeout=2)
            if s['owner'] != 'none' or s['commanded_output']['laser']:
                raise SafetyError('Initial state must be unowned and laser off')
            self.initial_mode = s['mode']
            motion=s.get('arm_motion',{})
            if 'speed_dps' in motion and 'acceleration_dps2' in motion:
                self.initial_arm_motion={k:motion[k] for k in ('speed_dps','acceleration_dps2')}
            keys = ('enabled','automatic','blink','swap','expression','brightness','tempo',
                    'play_mode','category','energy','rotate_left','rotate_right')
            self.initial_eyes = {k: s['eyes'][k] for k in keys}
            self.base = (s['commanded_output']['arm_left'], s['commanded_output']['arm_right'])
            if not s['commanded_output']['servo_enabled'] and not self.args.accepted_pose:
                raise SafetyError('No accepted pulsed pose: supervised setup required before soak')
            self.request('/control/mode', {'mode': 'auto'}, timeout=2)
            self.owns_mode = True
            if not s['commanded_output']['servo_enabled']:
                self.base = tuple(self.args.accepted_pose)
                self.status(timeout=2)
                self.session = self.request('/control/acquire', {'owner': 'program'})['session']
                self.last_ack = time.monotonic()
                self.command(*self.base, False)
                self.release()
                self.wait(.6)
                accepted = self.status(timeout=2)
                self.event('accepted_pose_seeded', status=accepted, reference=self.base)
            cycle = 0
            while time.monotonic() < self.deadline:
                names = list(SCENARIOS)
                if cycle:
                    self.rng.shuffle(names)
                for name in names:
                    if time.monotonic() >= self.deadline:
                        break
                    try:
                        self.run_scenario(name, self.args.scenario_seconds)
                    except SafetyError:
                        raise
                    except (urllib.error.URLError, TimeoutError, OSError) as e:
                        if isinstance(e, InterruptedError):
                            raise
                        self.counts['scenario_failures'] += 1
                        self.event('scenario_failure', error=str(e))
                        self.release()
                        self.wait(5)
                        # Bounded read-only recovery; next scenario starts with a NEW lease.
                        recovery_start = time.monotonic()
                        for attempt in range(6):
                            try:
                                recovered = self.status(timeout=2)
                                if recovered['owner'] != 'none' or recovered['commanded_output']['laser']:
                                    raise SafetyError('Recovery state is not idle')
                                self.event('recovery', seconds=time.monotonic()-recovery_start, attempt=attempt+1)
                                break
                            except SafetyError:
                                raise
                            except (urllib.error.URLError, TimeoutError, OSError):
                                self.wait(5)
                        else:
                            raise SafetyError('Device inaccessible after bounded recovery; no IP hammering')
                cycle += 1
            state = 'completed'
        except InterruptedError as e:
            state, error = 'cancelled', str(e)
        except Exception as e:
            error = str(e)
            self.event('fatal', error=error)
        finally:
            self.release()
            self.pool.shutdown(wait=True, cancel_futures=True)
            try:
                s = self.status(timeout=2)
                if s['owner'] == 'none' and not s['maintenance'] and self.owns_mode:
                    self.request('/eyes', self.initial_eyes, timeout=2)
                    if self.initial_arm_motion:
                        self.request('/arms/settings', self.initial_arm_motion, timeout=2)
                    self.request('/control/mode', {'mode': self.initial_mode}, timeout=2)
                    s = self.status(timeout=2)
                if s['commanded_output']['laser']:
                    raise SafetyError('Final laser is not off')
                self.event('final_idle', status=s)
            except Exception as e:
                self.event('final_idle_unverified', error=str(e))
                if state == 'completed':
                    state, error = 'failed', 'Final idle verification failed: '+str(e)
            self.checkpoint(state, force=True, error=error)
            self.log.close()
            self.csvfile.close()
        return 0 if state == 'completed' else 1


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--url', required=True)
    p.add_argument('--mac', required=True)
    p.add_argument('--version', required=True)
    p.add_argument('--build', required=True)
    p.add_argument('--hours', type=float, default=24)
    p.add_argument('--scenario-seconds', type=float, default=300)
    p.add_argument('--seed', type=int, default=20261016)
    p.add_argument('--accepted-pose', nargs=2, type=float,
                   help='Explicit previously accepted logical L/R reference for first pulses after OTA')
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    u = urllib.parse.urlparse(a.url)
    if u.scheme != 'http' or not u.hostname or u.path not in ('', '/'):
        p.error('Use an explicit verified http://host[:port] origin')
    if not 0 < a.hours <= 24 or not 1 <= a.scenario_seconds <= 900:
        p.error('Bound hours to (0,24], scenarios to [1,900] seconds')
    if a.accepted_pose and any(not math.isfinite(v) or not 0 <= v <= 180 for v in a.accepted_pose):
        p.error('Accepted pose must contain finite angles 0..180')
    with DeviceLock(a.mac.lower()):
        r = Runner(a)
        for sig in (signal.SIGINT, signal.SIGTERM):
            signal.signal(sig, lambda *_: r.cancel.set())
        raise SystemExit(r.run())


if __name__ == '__main__':
    main()
