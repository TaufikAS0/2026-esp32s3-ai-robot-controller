import pathlib
import sys
import time
import unittest
from unittest.mock import patch
import io
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / "04_Dokumen" / "python"))
from robot_client import RobotClient


class ClientTests(unittest.TestCase):
    def setUp(self):
        self.calls = []
        self.fail = False
        self.client = RobotClient("http://robot", transport=self.transport, target_ttl=0.2)

    def tearDown(self):
        self.client.close()

    def transport(self, path, body):
        self.calls.append((path, body))
        if path == "/command" and self.fail:
            raise OSError("offline")
        return {"session": 123} if path == "/control/acquire" else {"ok": True}

    def test_explicit_mode(self):
        self.client.set_mode("auto")
        self.assertIn(("/control/mode", {"mode": "auto"}), self.calls)
        with self.assertRaises(ValueError):
            self.client.set_mode("invalid")
        self.client.acquire()
        with self.assertRaises(RuntimeError):
            self.client.set_mode("manual")

    def test_http_status_requires_no_token(self):
        with RobotClient("http://robot") as client:
            with patch("urllib.request.urlopen", return_value=io.BytesIO(b'{"access_mode":"open_lan"}')) as http:
                self.assertEqual(client.status()["access_mode"], "open_lan")
                request = http.call_args.args[0]
                self.assertFalse(request.has_header("Authorization"))

    def test_acquisition_does_not_replay(self):
        self.client.acquire()
        time.sleep(0.25)
        self.assertFalse(any(p == "/command" for p, _ in self.calls))

    def test_producer_expiry_stops_background_replay(self):
        self.client.acquire()
        self.client.drive(0.4, -0.4)
        time.sleep(0.45)
        commands = [b for p, b in self.calls if p == "/command"]
        self.assertGreaterEqual(len(commands), 1)
        self.assertEqual(commands[0]["left"], 0.4)
        self.assertTrue(any(p == "/control/release" for p, _ in self.calls))
        count = len(commands)
        time.sleep(0.2)
        self.assertEqual(count, sum(p == "/command" for p, _ in self.calls))
        with self.assertRaises(RuntimeError):
            self.client.drive(0, 0)

    def test_fresh_updates_and_order(self):
        self.client.acquire()
        self.client.set_arms(10, 170)
        self.client.set_laser(True)
        for _ in range(6):
            self.client.drive(0.2, 0.3)
            time.sleep(0.05)
        self.client.stop()
        commands = [b for p, b in self.calls if p == "/command"]
        self.assertGreaterEqual(len(commands), 2)
        self.assertEqual([b["sequence"] for b in commands], list(range(1, len(commands) + 1)))
        self.assertEqual(commands[-1]["arm_right"], 170)
        self.assertTrue(commands[-1]["laser"])

    def test_validation_is_atomic(self):
        self.client.acquire()
        for left, right in [(float("nan"), 0), (0, 2), (True, 0)]:
            with self.assertRaises(ValueError):
                self.client.drive(left, right)
        with self.assertRaises(ValueError):
            self.client.set_arms(90, 181)
        with self.assertRaises(ValueError):
            self.client.set_laser(1)
        self.assertIsNone(self.client._updated_at)

    def test_network_error_releases_only_own_session(self):
        self.client.acquire()
        self.fail = True
        self.client.drive(0.2, 0.2)
        time.sleep(0.35)
        self.assertEqual(self.client.last_error, "offline")
        self.assertEqual(self.client._session, 0)
        self.assertFalse(any(p == "/stop" for p, _ in self.calls))
        self.assertTrue(any(p == "/control/release" for p, _ in self.calls))

    def test_blocked_io_cannot_send_stale_target(self):
        self.client.acquire()
        self.client._io.acquire()
        try:
            self.client.drive(0.4, 0.4)
            time.sleep(0.35)  # Heartbeat is waiting on transport; producer has expired.
        finally:
            self.client._io.release()
        time.sleep(0.2)
        self.assertFalse(any(p == "/command" for p, _ in self.calls))
        self.assertEqual(self.client._session, 0)


if __name__ == "__main__":
    unittest.main()
