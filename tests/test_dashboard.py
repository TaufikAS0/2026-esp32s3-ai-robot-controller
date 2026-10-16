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
        self.urls = []
        self.fast_channel = False
        self.drop_status = False
        self.arm_policy=dict(min_left=0,max_left=180,min_right=0,max_right=180,dynamic=False,amplitude_deg=8,period_ms=2400)
        self.session = 0
        self.mode = "manual"
        self.reject = False
        self.offline = False
        self.hardware = dict(chip_model="ESP32-S3", cpu_freq_mhz=240, free_heap_bytes=245760, heap_size_bytes=327680, min_free_heap_bytes=220000, psram_size_bytes=8388608, free_psram_bytes=8000000, application_bytes=1050000, ota_slot_bytes=6291456, flash_size_bytes=16777216, wifi_rssi_dbm=-55)
        self.eye_settings = dict(enabled=True, automatic=True, blink=True, swap=False,
                                 expression="happy", brightness=160, tempo=1,play_mode="reactive",category="all",energy=2,rotate_left=True,rotate_right=True)
        self.reject_eyes = False
        self.reject_checks = False
        self.errors = []
        self.page.on("pageerror", lambda e: self.errors.append(str(e)))
        self.page.route("http://robot*/**", self.route)
        self.page.goto("http://robot/")
        self.page.wait_for_function("document.getElementById('version').textContent===" + json.dumps(VERSION))

    def tearDown(self):
        self.assertEqual(self.errors, [])
        self.page.close()

    def route(self, route):
        req = route.request
        cors={"Access-Control-Allow-Origin":"http://robot","Access-Control-Allow-Headers":"Content-Type","Access-Control-Allow-Methods":"GET, POST, OPTIONS","Access-Control-Max-Age":"600"}
        if req.method=='OPTIONS':
            route.fulfill(status=204,headers=cors)
            return
        if req.url == "http://robot/":
            route.fulfill(status=200, content_type="text/html", body=HTML)
            return
        self.assertNotIn("authorization", req.headers)
        path = req.url.split("/api/v1", 1)[1]
        self.urls.append(req.url)
        body = json.loads(req.post_data) if req.post_data and "application/json" in req.headers.get("content-type", "") else None
        self.calls.append((path, body))
        if self.drop_status and path=='/status':
            route.abort('failed')
            return
        if self.offline:
            route.abort("failed")
            return
        code = 200
        if path == "/control/acquire":
            self.session = 42
            result = {"session": 42}
        elif path == "/control/mode":
            self.mode = body["mode"]
            self.session = 0
            result = {"ok": True}
        elif path in ("/stop", "/control/release"):
            self.session = 0
            result = {"ok": True}
        elif path == "/command" and self.reject:
            code = 409
            result = {"error": "session_or_sequence_rejected"}
        elif path == "/eyes/catalog":
            source=(ROOT / "02_Firmware/robot_controller/eye_catalog.h").read_text()
            entries=re.findall(r'X\(\w+,"([^"]+)","([^"]+)",(\w+),(\w+),(\d+),(\d+)\)',source)
            presets=[dict(id=id,label=label,category=category.lower(),shape=shape.lower(),
                     family='box' if shape in ('Solid','Happy','Focus','Sad') else 'special',
                     width=int(w),height=int(h)) for id,label,category,shape,w,h in entries]
            categories=[dict(id=c,label=c) for c in ['all']+list(dict.fromkeys(p['category'] for p in presets))]
            result=dict(presets=presets,categories=categories)
        elif path == "/arms/settings":
            result = {"ok":True}
        elif path == "/arms/policy":
            self.arm_policy=body
            result={"ok":True}
        elif path == "/eyes/check":
            code = 503 if self.reject_checks else 202
            result = {"error": "eye_bus_unavailable"} if self.reject_checks else {"ok": True, "queued": True}
        elif path == "/eyes":
            if self.reject_eyes:
                code = 400
                result = {"error": "invalid_eye_settings"}
            else:
                self.eye_settings = body
                result = {"ok": True}
        elif path == "/status":
            result = dict(firmware_version=VERSION, build_time_utc="2026-10-12T01:02:03Z", hardware=self.hardware, uptime_ms=112147, sta_connected=True, sta_ssid="HuaweiJIN", mode=self.mode, sta_ip="192.168.1.34", session=self.session, reason="stop", arduino_ota=False,
                          control_channel=dict(ready=self.fast_channel,port=81),arm_motion=dict(speed_dps=60,acceleration_dps2=240,**self.arm_policy),
                          eyes=dict(**self.eye_settings,task_ready=True,check=dict(mode="animate",target="both",scanned=112,scan_complete=True,found_addresses=[60],left_probe_code=2,right_probe_code=0,remaining_ms=0), active_expression="curious", reaction="turn_left", left_online=True, right_online=False,
                                    left=dict(x=48,y=32,width=50,height=40),right=dict(x=48,y=32,width=50,height=30)),
                          commanded_output=dict(left=0, right=0, arm_left=90, arm_right=90, laser=False))
        else:
            result = {"ok": True}
        route.fulfill(status=code, headers=cors,content_type="application/json", body=json.dumps(result))

    def test_slider_target_survives_release_and_poll(self):
        self.page.locator('#armLeft').fill('42')
        self.page.wait_for_function('controller.session===42&&!controller.pending')
        self.page.evaluate('async()=>{await controller.stop();await refresh();}')
        self.assertEqual(self.page.locator('#armLeft').input_value(),'42')
        self.assertEqual(self.page.locator('#armLeftNumber').input_value(),'42')
        self.assertIn('90.0°',self.page.locator('#armLeftActual').inner_text())

    def test_limits_ui_applies_and_bounds_sliders(self):
        self.page.get_by_text('Batas sudut & mode dinamis',exact=True).click()
        for id,value in [('armMinLeft','20'),('armMaxLeft','140'),('armMinRight','30'),('armMaxRight','150')]:
            self.page.locator('#'+id).fill(value)
        self.page.locator('#armDynamic').check()
        self.page.locator('#saveArmPolicy').click()
        self.page.wait_for_function("document.getElementById('armPolicyMessage').textContent.includes('diterapkan')")
        self.assertTrue(self.arm_policy['dynamic'])
        self.assertEqual(self.page.locator('#armLeft').get_attribute('min'),'20')
        self.assertEqual(self.page.locator('#armRight').get_attribute('max'),'150')
        self.assertFalse(any(p=='/command' for p,_ in self.calls))

    def test_fast_control_origin_and_port(self):
        self.fast_channel=True
        self.page.evaluate('refresh()')
        self.page.wait_for_function('fastControlReady')
        self.page.locator('#armRight').fill('80')
        self.page.wait_for_function('controller.session===42&&!controller.pending')
        self.assertTrue(any(u=='http://robot:81/api/v1/command' for u in self.urls))
        self.assertTrue(any(u=='http://robot:81/api/v1/control/acquire' for u in self.urls))

    def test_failed_status_does_not_release_fresh_control(self):
        self.acquire()
        self.page.wait_for_function('controller.lastAck>0&&!controller.pending')
        self.drop_status=True
        self.page.evaluate('refresh()')
        self.page.wait_for_timeout(60)
        self.assertEqual(self.page.evaluate('controller.session'),42)
        self.assertTrue(self.page.evaluate('connected'))

    def test_arm_motion_profile_and_sharingan(self):
        self.page.get_by_text('Kehalusan gerak tangan',exact=True).click()
        self.page.locator('#armMotionProfile').select_option('gentle')
        self.page.locator('#saveArmMotion').click()
        self.page.wait_for_function("document.getElementById('armMotionMessage').textContent.includes('diterapkan')")
        self.assertIn(('/arms/settings',{'speed_dps':25,'acceleration_dps2':60}),self.calls)
        self.page.locator('[data-eye-family="special"]').click()
        self.page.locator('[data-category="sharingan"]').click()
        self.page.locator('[data-expression="sharingan"]').click()
        self.page.wait_for_timeout(150)
        self.page.locator('#saveEyes').click()
        self.page.wait_for_function("!eyeBusy&&!eyeDirty")
        self.assertEqual(self.eye_settings['expression'],'sharingan')
        self.assertFalse(any(p in ('/command','/control/acquire') for p,_ in self.calls))

    def test_sharingan_variants_and_fox_silhouette(self):
        result=self.page.evaluate('''() => {
          const ids=['sharingan_1','sharingan_2','sharingan','mangekyo_pinwheel','mangekyo_flower','mangekyo_star'];
          const sheet=document.createElement('canvas');sheet.width=768;sheet.height=256;sheet.id='regressionSheet';
          sheet.style.cssText='position:fixed;top:0;left:0;z-index:99999;background:black';document.body.append(sheet);
          const ctx=sheet.getContext('2d'),canvas=document.createElement('canvas');canvas.width=128;canvas.height=64;
          let minDifference=8192;const frames=[];
          for(let phase=0;phase<4;phase++){
            const row=[];
            for(let i=0;i<ids.length;i++){
              paintEye(canvas,{x:64,y:32,width:52,height:50,phase:phase*.61},ids[i],true,true,255);
              ctx.drawImage(canvas,i*128,phase*64);
              const data=canvas.getContext('2d').getImageData(0,0,128,64).data;
              row.push(Array.from({length:8192},(_,k)=>data[k*4]>0?1:0));
            }
            for(let a=0;a<6;a++)for(let b=a+1;b<6;b++)minDifference=Math.min(minDifference,row[a].reduce((n,v,k)=>n+(v!==row[b][k]),0));
            frames.push(row);
          }
          let outside=0;
          for(const left of [false,true])for(let phase=0;phase<100;phase++){
            const p={x:64+Math.sin(phase*.3)*8,y:32,width:62,height:38,phase:phase*.17};
            paintEye(canvas,p,'fox',left,true,255);
            const data=canvas.getContext('2d').getImageData(0,0,128,64).data;
            for(let y=0;y<64;y++)for(let x=0;x<128;x++)if(data[(y*128+x)*4]>0){
              const dx=(x-p.x)/31,dy=(y-p.y)/19;
              if(dx*dx+dy*dy>=1.00001||dy<=(left?.55*dx:-.55*dx)-.40001)outside++;
            }
          }
          return {minDifference,outside,frames};
        }''')
        self.assertGreater(result['minDifference'],50)
        self.assertEqual(result['outside'],0)
        (ROOT/'build').mkdir(exist_ok=True)
        self.page.locator('#regressionSheet').screenshot(path=str(ROOT/'build/sharingan-browser.png'))
        self.page.evaluate('''() => {
          const sheet=document.getElementById('regressionSheet');sheet.width=256;sheet.height=256;
          const ctx=sheet.getContext('2d'),c=document.createElement('canvas');c.width=128;c.height=64;
          for(let phase=0;phase<4;phase++)for(let side=0;side<2;side++){
            paintEye(c,{x:64,y:32,width:62,height:38,phase:phase*.61},'fox',side===0,true,255);
            ctx.drawImage(c,side*128,phase*64);
          }
        }''')
        self.page.locator('#regressionSheet').screenshot(path=str(ROOT/'build/fox-browser.png'))
        native=ROOT/'build/sharingan-native.pgm'
        if native.exists():
            data=native.read_bytes().split(b'\n',3)[3]
            for phase,row in enumerate(result['frames']):
                for col,pixels in enumerate(row):
                    difference=sum(bool(data[(phase*64+y)*768+col*128+x])!=bool(pixels[y*128+x]) for y in range(64) for x in range(128))
                    self.assertLess(difference,8)

    def test_rotation_is_independent_from_swap(self):
        self.page.locator('#eyeRotateLeft').uncheck()
        self.page.locator('#eyeRotateRight').check()
        self.page.locator('#eyeSwap').check()
        self.page.locator('#saveEyes').click()
        self.page.wait_for_function("!eyeBusy&&!eyeDirty")
        self.assertFalse(self.eye_settings['rotate_left'])
        self.assertTrue(self.eye_settings['rotate_right'])
        self.assertTrue(self.eye_settings['swap'])
        self.page.wait_for_timeout(150)
        self.assertEqual(self.page.locator('#eyeLeftCanvas').evaluate('e=>e.style.transform'),'none')
        self.assertEqual(self.page.locator('#eyeRightCanvas').evaluate('e=>e.style.transform'),'rotate(180deg)')
        self.assertFalse(any(p in ('/command','/control/acquire') for p,_ in self.calls))

    def test_families_and_real_thumbnails(self):
        self.page.wait_for_selector('[data-expression="wink_left"] canvas')
        happy=self.page.locator('[data-expression="happy"] canvas').evaluate('e=>e.toDataURL()')
        laugh=self.page.locator('[data-expression="laugh"] canvas').evaluate('e=>e.toDataURL()')
        self.assertNotEqual(happy,laugh)
        self.assertEqual(self.page.locator('[data-expression="love"]').count(),0)
        self.assertGreater(self.page.locator('[data-expression] canvas').count(),20)
        self.page.locator('[data-eye-family="special"]').click()
        self.assertEqual(self.page.locator('[data-expression="love"]').count(),1)
        self.assertEqual(self.page.locator('[data-expression="neutral"]').count(),0)
        self.page.locator('#eyePlayMode').select_option('reactive')
        self.assertEqual(self.page.locator('[data-expression="love"]').count(),0)
        self.assertGreater(self.page.locator('[data-expression="wink_left"] canvas').count(),0)

    def test_catalog_categories_modes_and_winks(self):
        self.page.wait_for_selector('[data-expression="wink_left"]')
        self.assertLess(self.page.locator('[data-expression]').count(),41)
        self.assertGreater(self.page.locator('[data-expression]').count(),20)
        self.page.locator('[data-category="wink"]').click()
        self.assertEqual(self.page.locator('[data-expression]').count(),4)
        self.page.locator('[data-expression="wink_left"]').click()
        self.page.locator('#saveEyes').click()
        self.page.wait_for_function("!eyeBusy&&!eyeDirty")
        self.assertEqual(self.eye_settings['play_mode'],'manual')
        self.assertFalse(self.eye_settings['automatic'])
        self.page.locator('#eyePlayMode').select_option('showcase')
        self.page.locator('#eyeEnergy').fill('3')
        self.page.locator('#eyeEnergy').dispatch_event('input')
        self.page.locator('#saveEyes').click()
        self.page.wait_for_function("!eyeBusy&&!eyeDirty")
        self.assertEqual(self.eye_settings['category'],'wink')
        self.assertEqual(self.eye_settings['play_mode'],'showcase')
        self.assertEqual(self.eye_settings['energy'],3)
        self.assertFalse(any(p in ('/command','/control/acquire') for p,_ in self.calls))

    def test_oled_check_has_address_results_without_motor_lease(self):
        self.page.locator('#eyes-panel .eye-diag summary').click()
        self.assertIn('0x3C',self.page.locator('#eyeCheckResult').inner_text())
        self.page.locator('#eyeCheckTarget').select_option('left')
        self.page.locator('[data-eye-check="white"]').click()
        self.page.wait_for_function("!eyeCheckBusy")
        self.assertIn(('/eyes/check',{'mode':'white','target':'left'}),self.calls)
        self.assertFalse(any(p in ('/command','/control/acquire') for p,_ in self.calls))

    def test_oled_check_rejection_retains_controls(self):
        self.reject_checks = True
        self.page.locator('#eyes-panel .eye-diag summary').click()
        self.page.locator('[data-eye-check="scan"]').click()
        self.page.wait_for_function("document.getElementById('eyeCheckMessage').textContent.includes('Tes gagal')")
        self.assertIn('eye_bus_unavailable', self.page.locator('#eyeCheckMessage').inner_text())
        self.assertFalse(self.page.locator('[data-eye-check="scan"]').is_disabled())

    def test_eye_settings_swap_and_no_motor_lease(self):
        self.page.locator('[data-eye-family="special"]').click()
        self.page.locator('[data-expression="love"]').click()
        self.page.locator('#eyeSwap').check()
        self.page.locator('#eyeBrightness').fill("120")
        self.page.locator('#eyeBrightness').dispatch_event("input")
        self.page.locator('#saveEyes').click()
        self.page.wait_for_function("document.getElementById('eyeMessage').textContent.includes('Diterapkan')")
        body = [body for path, body in self.calls if path == "/eyes"][-1]
        self.assertFalse(body["automatic"])
        self.assertTrue(body["swap"])
        self.assertEqual(body["expression"], "love")
        self.assertEqual(body["brightness"], 120)
        self.assertFalse(any(path in ("/command", "/control/acquire") for path, _ in self.calls))

    def test_eye_draft_survives_poll_and_error(self):
        self.page.locator('[data-expression="sleepy"]').click()
        self.page.evaluate("refresh()")
        self.page.wait_for_timeout(200)
        self.assertTrue(self.page.locator('[data-expression="sleepy"]').evaluate("e=>e.classList.contains('selected')"))
        self.reject_eyes = True
        self.page.locator('#saveEyes').click()
        self.page.wait_for_function("document.getElementById('eyeMessage').textContent.includes('Gagal')")
        self.assertTrue(self.page.evaluate("eyeDirty"))
        self.page.locator('#resetEyes').click()
        self.assertFalse(self.page.evaluate("eyeDirty"))

    def test_eye_missing_panel_and_mobile_layout(self):
        self.assertIn("ACK I²C",self.page.locator('#eyeLeftStatus').inner_text())
        self.assertIn("tidak terdeteksi",self.page.locator('#eyeRightStatus').inner_text())
        self.page.set_viewport_size({"width":390,"height":844})
        self.assertTrue(self.page.evaluate("document.documentElement.scrollWidth<=innerWidth"))
        self.page.locator('[data-eye-family="special"]').click()
        self.page.locator('[data-expression="love"]').click()
        self.page.wait_for_timeout(200)
        self.assertTrue(self.page.locator('#eyeLeftCanvas').evaluate("e=>e.getContext('2d').getImageData(0,0,128,64).data.some(v=>v>0)"))
        self.page.evaluate("renderEyeStatus(undefined,false)")
        self.assertTrue(self.page.locator('#saveEyes').is_disabled())
        self.assertIn("BELUM MENDUKUNG",self.page.locator('#eyeLive').inner_text())

    def test_telemetry_values_and_offline_label(self):
        self.assertEqual(self.page.locator("#heapMetric").inner_text(), "240.0 KiB / 320.0 KiB")
        self.assertEqual(self.page.locator("#heapBar").evaluate("el=>el.style.width"), "75%")
        self.assertEqual(self.page.locator("#rssiInfo").inner_text(), "-55 dBm")
        self.assertIn("16.00 MiB", self.page.locator("#flashInfo").inner_text())
        self.offline = True
        self.page.wait_for_function("!connected")
        self.assertIn("data terakhir", self.page.locator("#statusBadge").inner_text())

    def test_navigation_opens_wifi_and_offline_assets(self):
        self.page.locator("#wifiNav").click()
        self.assertTrue(self.page.locator("#wifi-settings").evaluate("el=>el.open"))
        self.assertFalse(self.page.locator('script[src],link[href^="http"],img[src^="http"]').count())
        self.assertNotIn("WebSockets", HTML)
        self.assertNotIn("ESP32-WROOM", HTML)

    def test_missing_telemetry_is_not_fabricated(self):
        self.hardware = {}
        self.page.evaluate("refresh()")
        self.page.wait_for_function("document.getElementById('heapMetric').textContent==='Tidak tersedia'")
        self.assertEqual(self.page.locator("#heapBar").evaluate("el=>el.style.width"), "0%")

    def test_build_time_visible(self):
        self.assertEqual(self.page.locator("#buildTime").inner_text(), "Build UTC: 2026-10-12T01:02:03Z")

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
        button = self.page.locator('[data-drive="forward"]')
        button.hover()
        self.page.mouse.down()
        self.page.wait_for_timeout(180)
        self.assertTrue(any(p == "/command" and b["left"] > 0 for p, b in self.calls))
        self.page.mouse.up()
        self.assert_stopped()
        self.assertTrue(any(p in ("/stop", "/control/release") for p, _ in self.calls))

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

    def test_open_access_and_ota_upload(self):
        self.assertEqual(self.page.locator("#token").count(), 0)
        self.acquire()
        self.page.locator("#firmware").set_input_files({"name": "robot.bin", "mimeType": "application/octet-stream", "buffer": b"test-image"})
        self.page.locator("#upload").click()
        self.page.wait_for_function("document.getElementById('message').textContent.includes('Upload berhasil')")
        self.assertTrue(any(p in ("/stop", "/control/release") for p, _ in self.calls))
        self.assertTrue(any(p == "/update" for p, _ in self.calls))
        self.assert_stopped()

    def test_release_before_acquire_response_never_commands(self):
        result = self.page.evaluate("""async () => {
            let reply; const calls=[];
            const fakeRequest=(path,body)=>{calls.push(path);return path==='/control/acquire'?new Promise(resolve=>{reply=resolve;}):Promise.resolve({ok:true});};
            const c=new RobotDashboard(fakeRequest,()=>{});
            c.desired.left=c.desired.right=.3;
            const pending=c.acquire(); await c.stop(); reply({session:99}); await pending;
            return {calls,session:c.session,timer:c.timer};
        }""")
        self.assertEqual(result["session"], 0)
        self.assertIsNone(result["timer"])
        self.assertNotIn("/command", result["calls"])
        self.assertIn("/control/release", result["calls"])

    def test_connects_on_load(self):
        self.assertIn("Terhubung", self.page.locator("#connection").inner_text())
        self.assertTrue(any(p == "/status" for p, _ in self.calls))

    def test_auto_mode_blocks_dashboard_and_manual_returns(self):
        self.acquire()
        self.page.locator("#modeAuto").click()
        self.page.wait_for_function("mode==='auto' && controller.session===0")
        self.assertFalse(self.page.locator('[data-drive="forward"]').is_enabled())
        self.assertFalse(self.page.locator("#armLeft").is_enabled())
        self.page.locator("#modeManual").click()
        self.page.wait_for_function("mode==='manual'")
        self.assertTrue(self.page.locator('[data-drive="forward"]').is_enabled())

    def test_disconnect_recovers_without_click(self):
        self.offline = True
        self.page.wait_for_function("!connected")
        self.assertIn("Tidak terhubung", self.page.locator("#connection").inner_text())
        self.offline = False
        self.page.wait_for_function("connected")

    def test_hand_control_acquires_automatically(self):
        self.page.locator("#armLeft").fill("25")
        self.page.wait_for_function("controller.session===42")
        self.page.wait_for_timeout(150)
        self.assertTrue(any(p == "/command" and b["arm_left"] == 25 for p, b in self.calls))

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
