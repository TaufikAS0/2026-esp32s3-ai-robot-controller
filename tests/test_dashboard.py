"""Chromium tests against the actual embedded dashboard, with mocked device HTTP."""
import json
import os
import pathlib
import re
import unittest
from playwright.sync_api import sync_playwright

ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "02_Firmware/robot_controller/web_ui.h").read_text(encoding="utf-8")
HTML = SOURCE.split('R"HTML(', 1)[1].split(')HTML";', 1)[0]
VERSION = re.search(r'kFirmwareVersion\[\]\s*=\s*"([^"]+)"',
                    (ROOT / "02_Firmware/robot_controller/firmware_version.h").read_text()).group(1)


class DashboardTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.playwright = sync_playwright().start()
        cls.browser = cls.playwright.chromium.launch(
            executable_path=os.environ.get("PLAYWRIGHT_CHROMIUM_EXECUTABLE"))

    @classmethod
    def tearDownClass(cls):
        cls.browser.close()
        cls.playwright.stop()

    def setUp(self):
        self.page = self.browser.new_page(viewport={"width": 1100, "height": 1100})
        self.calls = []
        self.session = 0
        self.reject = False
        self.offline = False
        self.errors = []
        self.page.on("pageerror", lambda e: self.errors.append(str(e)))
        self.page.route("http://robot/**", self.route)
        self.page.goto("http://robot/")
        self.page.locator("#token").fill("test-only")
        self.page.locator("#connect").click()
        self.page.wait_for_function("document.getElementById('version').textContent===" + json.dumps(VERSION))

    def tearDown(self):
        self.assertEqual(self.errors, [])
        self.page.close()

    def route(self, route):
        req = route.request
        if req.url == "http://robot/":
            route.fulfill(status=200, content_type="text/html", body=HTML)
            return
        path = req.url.split("/api/v1", 1)[1]
        body = json.loads(req.post_data) if req.post_data and "application/json" in req.headers.get("content-type", "") else None
        self.calls.append((path, body))
        if self.offline:
            route.abort("failed")
            return
        code = 200
        if path == "/control/acquire":
            self.session = 42
            result = {"session": 42}
        elif path in ("/stop", "/control/release"):
            self.session = 0
            result = {"ok": True}
        elif path == "/command" and self.reject:
            code = 409
            result = {"error": "session_or_sequence_rejected"}
        elif path == "/status":
            result = dict(firmware_version=VERSION, session=self.session, reason="stop", arduino_ota=False,
                          commanded_output=dict(left=0, right=0, arm_left=90, arm_right=90, laser=False))
        else:
            result = {"ok": True}
        route.fulfill(status=code, content_type="application/json", body=json.dumps(result))

    def acquire(self):
        self.page.locator("#acquire").click()
        self.page.wait_for_function("controller.session===42")

    def assert_stopped(self):
        self.page.wait_for_function("controller.session===0 && controller.timer===null")
        self.page.wait_for_timeout(150)
        count = sum(p == "/command" for p, _ in self.calls)
        self.page.wait_for_timeout(250)
        self.assertEqual(count, sum(p == "/command" for p, _ in self.calls))

    def test_pointer_release_stops(self):
        self.acquire()
        button = self.page.locator('[data-drive="forward"]')
        button.hover()
        self.page.mouse.down()
        self.page.wait_for_timeout(180)
        self.assertTrue(any(p == "/command" and b["left"] > 0 for p, b in self.calls))
        self.page.mouse.up()
        self.assert_stopped()
        self.assertTrue(any(p == "/stop" for p, _ in self.calls))

    def test_focus_loss_stops(self):
        self.acquire()
        self.page.evaluate("window.dispatchEvent(new Event('blur'))")
        self.assert_stopped()

    def test_hidden_tab_stops(self):
        self.acquire()
        self.page.evaluate("Object.defineProperty(document,'hidden',{configurable:true,value:true});document.dispatchEvent(new Event('visibilitychange'))")
        self.assert_stopped()

    def test_network_failure_stops_retries(self):
        self.acquire()
        self.offline = True
        self.assert_stopped()

    def test_api_rejection_stops(self):
        self.acquire()
        self.reject = True
        self.assert_stopped()

    def test_stop_button_and_arm_controls(self):
        self.acquire()
        self.page.locator("#armLeft").fill("35")
        self.page.locator("#laser").check()
        self.page.wait_for_timeout(180)
        self.assertTrue(any(p == "/command" and b["arm_left"] == 35 and b["laser"] for p, b in self.calls))
        self.page.locator("#stop").click()
        self.assert_stopped()
        self.assertFalse(self.page.locator("#laser").is_checked())

    def test_mobile_layout(self):
        self.page.set_viewport_size({"width": 390, "height": 844})
        self.assertLessEqual(self.page.evaluate("document.documentElement.scrollWidth"), 390)
        self.page.screenshot(path=str(ROOT / "build/dashboard-mobile.png"), full_page=True)

    def test_desktop_layout(self):
        self.assertLessEqual(self.page.evaluate("document.documentElement.scrollWidth"), 1100)
        self.page.screenshot(path=str(ROOT / "build/dashboard-desktop.png"), full_page=True)


if __name__ == "__main__":
    (ROOT / "build").mkdir(exist_ok=True)
    unittest.main()
