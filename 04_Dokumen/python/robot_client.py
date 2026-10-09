"""Standard-library robot client. Producer updates expire independently of HTTP heartbeat."""
from __future__ import annotations
import json
import math
import threading
import time
import urllib.request


class RobotClient:
    def __init__(self, base_url: str, token: str | None = None, *, target_ttl: float = 0.3,
                 timeout: float = 0.25, transport=None):
        if not 0.1 <= target_ttl <= 0.4:
            raise ValueError("target_ttl must be 0.1..0.4 seconds")
        # Legacy token argument is accepted for older scripts, ignored and never sent.
        self.base_url = base_url.rstrip("/")
        self.target_ttl = target_ttl
        self.timeout = timeout
        self._transport = transport
        self._lock = threading.RLock()
        self._io = threading.Lock()
        self._closed = threading.Event()
        self._session = 0
        self._sequence = 0
        self._updated_at = None
        self._acquired_at = None
        self._desired = dict(left=0.0, right=0.0, arm_left=90.0, arm_right=90.0, laser=False)
        self.last_error = None
        self._thread = threading.Thread(target=self._worker, daemon=True, name="robot-heartbeat")
        self._thread.start()

    def _request(self, path, body=None):
        # Serializes requests; state locks are never held during network I/O.
        with self._io:
            if path == "/command":
                # Recheck after waiting for another HTTP request. An I/O queue must
                # never extend an old producer target's lifetime.
                with self._lock:
                    if (body["session"] != self._session or self._updated_at is None or
                            time.monotonic() - self._updated_at >= self.target_ttl):
                        raise RuntimeError("producer_target_expired")
                    body.update(self._desired)
            if self._transport:
                return self._transport(path, body)
            data = None if body is None else json.dumps(body, allow_nan=False).encode()
            req = urllib.request.Request(self.base_url + "/api/v1" + path, data=data,
                                         method="GET" if body is None else "POST",
                                         headers={"Content-Type": "application/json"})
            with urllib.request.urlopen(req, timeout=self.timeout) as response:
                return json.load(response)

    def set_mode(self, mode):
        if mode not in ("auto", "manual"):
            raise ValueError("mode must be auto or manual")
        with self._lock:
            if self._session:
                raise RuntimeError("release current session before changing mode")
        return self._request("/control/mode", {"mode": mode})

    def acquire(self, owner="program"):
        if owner not in ("manual", "program"):
            raise ValueError("owner must be manual or program")
        with self._lock:
            if self._closed.is_set():
                raise RuntimeError("client closed")
            if self._session:
                raise RuntimeError("release current session first")
        result = self._request("/control/acquire", {"owner": owner})
        with self._lock:
            self._session = result["session"]
            self._sequence = 0
            self._updated_at = None  # Acquisition alone never starts motion/heartbeats.
            self._acquired_at = time.monotonic()
            self._desired.update(left=0.0, right=0.0, laser=False)
            self.last_error = None
        return self._session

    @staticmethod
    def _number(value, low, high):
        if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or not low <= value <= high:
            raise ValueError(f"value must be finite in {low}..{high}")
        return float(value)

    def _set(self, **values):
        with self._lock:
            if self._closed.is_set() or not self._session:
                raise RuntimeError("acquire control first")
            self._desired.update(values)
            self._updated_at = time.monotonic()

    def drive(self, left, right):
        self._set(left=self._number(left, -1, 1), right=self._number(right, -1, 1))

    def set_arms(self, left, right):
        self._set(arm_left=self._number(left, 0, 180), arm_right=self._number(right, 0, 180))

    def set_laser(self, enabled):
        if type(enabled) is not bool:
            raise ValueError("laser must be boolean")
        self._set(laser=enabled)

    def _invalidate(self, session):
        with self._lock:
            if self._session == session:
                self._session = 0
                self._updated_at = None
                self._desired.update(left=0.0, right=0.0, laser=False)

    def _worker(self):
        while not self._closed.wait(0.1):
            with self._lock:
                session = self._session
                updated = self._updated_at
                if not session:
                    continue
                stale = time.monotonic() - (updated if updated is not None else self._acquired_at) >= self.target_ttl
                if updated is None and not stale:
                    continue
                if not stale:
                    self._sequence += 1
                    body = dict(session=session, sequence=self._sequence, **self._desired)
            if stale:
                self._invalidate(session)
                try:
                    self._request("/control/release", {"session": session})
                except Exception as exc:
                    self.last_error = str(exc)
                continue
            try:
                self._request("/command", body)
            except Exception as exc:
                self.last_error = str(exc)
                self._invalidate(session)
                # Release only our lease: never stop a newer manual owner on an old-session error.
                try:
                    self._request("/control/release", {"session": session})
                except Exception:
                    pass

    def stop(self):
        with self._lock:
            session = self._session
            self._invalidate(session)
        return self._request("/stop", {})

    def release(self):
        with self._lock:
            session = self._session
            self._invalidate(session)
        if session:
            return self._request("/control/release", {"session": session})

    def status(self):
        return self._request("/status")

    def close(self):
        self._closed.set()
        self._thread.join(timeout=2)
        self.release()

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.close()
