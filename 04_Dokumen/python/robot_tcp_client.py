"""Synchronous TCP v1 client. No heartbeat thread, reconnect, retry or stale replay."""
import json
import math
import socket
import threading
import time


class RobotTcpError(RuntimeError):
    pass


class RobotTcpClient:
    def __init__(self, host, port=9000, *, timeout=0.2, target_ttl=0.3):
        if not 0 < timeout <= 0.25 or not 0 < target_ttl <= 0.3:
            raise ValueError('timeout<=0.25 and target_ttl<=0.3 seconds required')
        self.host, self.port = host, port
        self.timeout, self.target_ttl = timeout, target_ttl
        self._socket = None
        self._buffer = bytearray()
        self._lock = threading.RLock()
        self._id = self.session = self.sequence = 0

    def connect(self):
        with self._lock:
            if self._socket is not None:
                raise RuntimeError('Already connected')
            self._socket = socket.create_connection((self.host, self.port), self.timeout)
            self._socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            self._buffer.clear()
            self._id = self.session = self.sequence = 0
            try:
                hello = self.request('hello')
                if hello.get('protocol_version') != 1 or hello.get('timeout_ms') != 500:
                    raise RobotTcpError('Incompatible protocol or timeout')
            except Exception:
                self.close()
                raise
            return hello

    def close(self):
        # Closing the socket releases only the connection-bound firmware lease.
        with self._lock:
            sock, self._socket = self._socket, None
            self.session = self.sequence = 0
            self._buffer.clear()
            if sock is not None:
                try:
                    sock.shutdown(socket.SHUT_RDWR)
                except OSError:
                    pass
                sock.close()

    def request(self, kind, **fields):
        with self._lock:
            if self._socket is None:
                raise RuntimeError('Connect explicitly before sending')
            if self._id >= 4294967295:
                self.close()
                raise RobotTcpError('Request ID exhausted; reconnect explicitly')
            self._id += 1
            payload = json.dumps(dict(v=1, id=self._id, type=kind, **fields),
                                 allow_nan=False, separators=(',', ':')).encode('utf-8')
            if len(payload) > 1024:
                raise ValueError('Request exceeds 1024 bytes')
            deadline = time.monotonic() + self.timeout
            try:
                self._socket.settimeout(max(0.001, deadline - time.monotonic()))
                self._socket.sendall(payload + b'\n')
                while b'\n' not in self._buffer:
                    remaining = deadline - time.monotonic()
                    if remaining <= 0:
                        raise TimeoutError('Reply deadline exceeded')
                    self._socket.settimeout(remaining)
                    chunk = self._socket.recv(4096)
                    if not chunk:
                        raise ConnectionError('Robot disconnected')
                    self._buffer.extend(chunk)
                    if len(self._buffer) > 8192:
                        raise RobotTcpError('Reply exceeds 8192 bytes')
                line, _, trailing = self._buffer.partition(b'\n')
                self._buffer = bytearray(trailing)
                reply = json.loads(line.decode('utf-8'))
                if (not isinstance(reply, dict) or type(reply.get('id')) is not int or
                        reply['id'] != self._id or type(reply.get('ok')) is not bool):
                    raise RobotTcpError('Invalid or mismatched reply')
                if time.monotonic() >= deadline:
                    raise TimeoutError('Reply deadline exceeded')
            except (OSError, ValueError, RobotTcpError):
                self.close()
                raise
            if not reply['ok']:
                raise RobotTcpError(reply.get('error', 'request_rejected'))
            if not isinstance(reply.get('data'), dict):
                self.close()
                raise RobotTcpError('Missing response data')
            return reply['data']

    def status(self):
        return self.request('status')

    def set_mode(self, mode):
        if mode not in ('auto', 'manual'):
            raise ValueError('mode must be auto or manual')
        return self.request('mode', mode=mode)

    def acquire(self):
        if self.session:
            raise RuntimeError('Release the current session first')
        result = self.request('acquire')
        session = result.get('session')
        if type(session) is not int or not 1 <= session <= 4294967295:
            self.close()
            raise RobotTcpError('Invalid acquired session')
        self.session, self.sequence = session, 0
        return session

    def command(self, *, left, right, arm_left, arm_right, laser=False, produced_at):
        """Send one fresh complete engine target; produced_at uses time.monotonic()."""
        with self._lock:
            try:
                if not self.session:
                    raise RuntimeError('Acquire explicitly before commanding')
                for name, value, lo, hi in [('left', left, -1, 1), ('right', right, -1, 1),
                                            ('arm_left', arm_left, 0, 180), ('arm_right', arm_right, 0, 180)]:
                    if type(value) not in (int, float) or not math.isfinite(value) or not lo <= value <= hi:
                        raise ValueError('Invalid ' + name)
                if type(laser) is not bool:
                    raise ValueError('laser must be boolean')
                if type(produced_at) not in (int, float) or not math.isfinite(produced_at):
                    raise ValueError('Invalid producer timestamp')
                age = time.monotonic() - produced_at
                if not 0 <= age < self.target_ttl:
                    raise TimeoutError('Engine target expired or timestamp is in the future')
                if self.sequence >= 4294967295:
                    raise RobotTcpError('Sequence exhausted; release and acquire explicitly')
                self.sequence += 1
                return self.request('command', session=self.session, sequence=self.sequence,
                                    left=left, right=right, arm_left=arm_left, arm_right=arm_right, laser=laser)
            except Exception:
                self.close()
                raise

    def release(self):
        if self.session:
            try:
                return self.request('release', session=self.session)
            finally:
                self.session = self.sequence = 0

    def stop(self):
        try:
            return self.request('stop')  # Explicit GLOBAL stop, like HTTP /stop.
        finally:
            self.session = self.sequence = 0

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, *args):
        self.close()
