import json
from pathlib import Path
import socket
import sys
import threading
import time
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / '04_Dokumen/python'))
from robot_tcp_client import RobotTcpClient, RobotTcpError


class Server:
    def __init__(self, transform=None):
        self.listener = socket.socket()
        self.listener.bind(('127.0.0.1', 0))
        self.listener.listen(1)
        self.listener.settimeout(1)
        self.port = self.listener.getsockname()[1]
        self.requests = []
        self.transform = transform
        self.closed = threading.Event()
        self.thread = threading.Thread(target=self.run, daemon=True)
        self.thread.start()

    def run(self):
        try:
            conn, _ = self.listener.accept()
            with conn:
                data = bytearray()
                while True:
                    chunk = conn.recv(4096)
                    if not chunk:
                        break
                    data.extend(chunk)
                    while b'\n' in data:
                        line, _, data = data.partition(b'\n')
                        r = json.loads(line)
                        self.requests.append(r)
                        payload = {'protocol_version': 1, 'timeout_ms': 500} if r['type'] == 'hello' else {'session': 123} if r['type'] == 'acquire' else {}
                        reply = dict(id=r['id'], ok=True, data=payload)
                        if self.transform:
                            reply = self.transform(r, reply)
                        if reply is None:
                            time.sleep(.35)
                            continue
                        encoded = (json.dumps(reply) + '\n').encode()
                        # Actual socket fragmentation, not a mocked recv.
                        conn.sendall(encoded[:3])
                        conn.sendall(encoded[3:])
        except (OSError, ValueError):
            pass
        finally:
            self.listener.close()
            self.closed.set()

    def finish(self):
        self.listener.close()
        self.thread.join(1)


class TcpClientTests(unittest.TestCase):
    def create(self, transform=None):
        server = Server(transform)
        client = RobotTcpClient('127.0.0.1', server.port)
        self.addCleanup(server.finish)
        self.addCleanup(client.close)
        client.connect()
        return server, client

    def target(self, client, **overrides):
        values = dict(left=0, right=0, arm_left=42, arm_right=79, laser=False, produced_at=time.monotonic())
        values.update(overrides)
        return client.command(**values)

    def test_discovery_never_acquires_or_commands(self):
        server, client = self.create()
        client.status()
        self.assertEqual([r['type'] for r in server.requests], ['hello', 'status'])

    def test_explicit_lifecycle_and_fragmented_replies(self):
        server, client = self.create()
        client.set_mode('auto')
        client.acquire()
        self.target(client)
        self.target(client)
        client.release()
        commands = [r for r in server.requests if r['type'] == 'command']
        self.assertEqual([r['sequence'] for r in commands], [1, 2])
        self.assertTrue(all(r['left'] == r['right'] == 0 and r['laser'] is False for r in commands))
        self.assertEqual(client.session, 0)

    def test_idle_does_not_repeat_target(self):
        server, client = self.create()
        client.acquire()
        self.target(client)
        time.sleep(.12)
        self.assertEqual(len([r for r in server.requests if r['type'] == 'command']), 1)

    def test_stale_target_disconnects_without_transmitting(self):
        server, client = self.create()
        client.acquire()
        with self.assertRaises(TimeoutError):
            self.target(client, produced_at=time.monotonic() - .4)
        self.assertFalse(any(r['type'] == 'command' for r in server.requests))
        self.assertIsNone(client._socket)
        self.assertTrue(server.closed.wait(1))

    def test_future_target_disconnects(self):
        server, client = self.create()
        client.acquire()
        with self.assertRaises(TimeoutError):
            self.target(client, produced_at=time.monotonic() + 10)
        self.assertFalse(any(r['type'] == 'command' for r in server.requests))

    def test_bad_target_does_not_renew(self):
        for overrides in [dict(left=True), dict(right=float('nan')), dict(arm_left=181), dict(laser=1)]:
            with self.subTest(overrides=overrides):
                server, client = self.create()
                client.acquire()
                with self.assertRaises(ValueError):
                    self.target(client, **overrides)
                self.assertFalse(any(r['type'] == 'command' for r in server.requests))
                self.assertIsNone(client._socket)

    def test_timeout_never_retries_or_reconnects(self):
        server, client = self.create(lambda r, reply: None if r['type'] == 'command' else reply)
        client.acquire()
        with self.assertRaises((TimeoutError, OSError)):
            self.target(client)
        self.assertEqual(len([r for r in server.requests if r['type'] == 'command']), 1)
        self.assertEqual(client.session, 0)
        with self.assertRaises(RuntimeError):
            self.target(client)

    def test_wrong_reply_id_disconnects(self):
        server, client = self.create(lambda r, reply: dict(reply, id=999) if r['type'] == 'status' else reply)
        with self.assertRaises(RobotTcpError):
            client.status()
        self.assertIsNone(client._socket)

    def test_command_rejection_disconnects(self):
        server, client = self.create(lambda r, reply: dict(id=r['id'], ok=False, error='session_or_sequence_rejected') if r['type'] == 'command' else reply)
        client.acquire()
        with self.assertRaisesRegex(RobotTcpError, 'session_or_sequence_rejected'):
            self.target(client)
        self.assertIsNone(client._socket)

    def test_mode_rejection_does_not_send_global_stop(self):
        server, client = self.create(lambda r, reply: dict(id=r['id'], ok=False, error='release_control_before_mode') if r['type'] == 'mode' else reply)
        with self.assertRaises(RobotTcpError):
            client.set_mode('auto')
        self.assertFalse(any(r['type'] == 'stop' for r in server.requests))

    def test_sequence_exhaustion_no_wrap(self):
        server, client = self.create()
        client.acquire()
        client.sequence = 4294967295
        with self.assertRaises(RobotTcpError):
            self.target(client)
        self.assertFalse(any(r['type'] == 'command' for r in server.requests))


if __name__ == '__main__':
    unittest.main()
