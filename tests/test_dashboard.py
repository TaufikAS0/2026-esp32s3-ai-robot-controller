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
        self.network_busy=False
        self.reject_wifi=False
        self.wifi=dict(storage_ready=True,state='connected',connected=True,current_ssid='HuaweiJIN',target_ssid='HuaweiJIN',sta_ip='192.168.1.22',ap_active=True,ap_ssid='robot-setup',ap_ip='192.168.4.1',rssi=-55,busy=False,scan_state='idle',networks=[],profiles=[dict(ssid='HuaweiJIN',priority=1,secured=True)])
        self.demo=dict(ready=True,storage_ready=True,running=False,simulation=False,calibrating=False,zero_valid=False,calibration_ready=False,can_start_robot=False,run_id=0,sample_id=0,phase='idle',reason='boot',estimated=True,pose=dict(x_cm=0,y_cm=0,heading_deg=0),goal=dict(x_cm=0,y_cm=0),trace=[],config=dict(width_cm=0,length_cm=0,radius_cm=0,margin_cm=10,coast_cm=0,speed_cm_s=5,turn_deg_s=30,pwm=.15,base_left=90,base_right=90,min_left=0,max_left=180,min_right=0,max_right=180,arms_confirmed=False,calibration=0))
        self.arm_policy=dict(min_left=0,max_left=180,min_right=0,max_right=180,dynamic=False,amplitude_deg=8,period_ms=2400)
        self.demo['exhibition']=dict(selected=False,center_valid=False,can_start=False,width_cm=30,length_cm=30)
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
        elif path == '/demo':
            result=self.demo
        elif path.startswith('/demo/'):
            result=dict(ok=True)
            if path=='/demo/exhibition/config':
                self.demo['exhibition'].update(**body,selected=True,center_valid=False,can_start=False)
            elif path=='/demo/exhibition/center':
                self.demo['exhibition'].update(selected=True,center_valid=True,can_start=True)
                self.demo.update(pose=dict(x_cm=0,y_cm=0,heading_deg=0),trace=[],zero_valid=False)
            elif path=='/demo/exhibition/start':
                self.demo['exhibition'].update(selected=True,center_valid=False,can_start=False)
                self.demo.update(running=True,run_id=self.demo['run_id']+1,phase='drive',simulation=body['mode']=='simulation')
            elif path=='/demo/config':
                self.demo['config'].update(body)
            elif path=='/demo/zero':
                self.demo['zero_valid']=True
            elif path=='/demo/start':
                self.demo.update(running=True,run_id=self.demo['run_id']+1,phase='drive',simulation=body['mode']=='simulation')
            elif path=='/demo/stop':
                if body['run_id']!=self.demo['run_id']:code=409;result=dict(error='demo_run_mismatch')
                else:
                    self.demo.update(running=False,zero_valid=False,phase='idle',sample_pending=False,sample_reason='demo_stop')
                    self.demo['exhibition'].update(center_valid=False,can_start=False)
            elif path=='/demo/calibration/start':
                self.demo.update(running=True,calibrating=True,run_id=8,sample_id=8,phase='calibration',zero_valid=False,sample_pending=False,sample_kind=['forward','cw','ccw'].index(body['kind']))
            elif path=='/demo/calibration/result':
                if self.demo.get('sample_pending'):
                    self.demo['config']['calibration']|=1<<self.demo['sample_kind']
                    self.demo.update(sample_pending=False,sample_reason='result_saved',zero_valid=False)
        elif path == '/wifi':
            result=self.wifi
        elif path.startswith('/wifi/') or (path=='/settings' and 'ssid' in body):
            code=409 if self.reject_wifi else 200
            result=dict(error='release_control_or_wait_for_network') if self.reject_wifi else dict(ok=True)
            if not self.reject_wifi:
                if path=='/wifi/scan':
                    self.wifi['scan_state']='complete'
                    self.wifi['networks']=[dict(ssid='Backup WiFi',rssi=-48,secured=True),dict(ssid='<img src=x onerror=alert(1)>',rssi=-60,secured=False)]
                elif path in ('/wifi/save','/settings'):
                    self.wifi['profiles']=[p for p in self.wifi['profiles'] if p['ssid']!=body['ssid']]+[dict(ssid=body['ssid'],priority=2,secured=bool(body['password']))]
                elif path=='/wifi/order':
                    self.wifi['profiles']=sorted(self.wifi['profiles'],key=lambda p:body['ssids'].index(p['ssid']))
                elif path=='/wifi/delete':
                    self.wifi['profiles']=[p for p in self.wifi['profiles'] if p['ssid']!=body['ssid']]
        elif path == "/status":
            result = dict(firmware_version=VERSION, build_time_utc="2026-10-12T01:02:03Z", hardware=self.hardware, uptime_ms=112147, sta_connected=True, sta_ssid="HuaweiJIN", mode=self.mode, sta_ip="192.168.1.34", session=self.session, reason="stop", arduino_ota=False,
                          owner='manual' if self.session else 'none',maintenance=False,network_busy=self.network_busy,control_channel=dict(ready=self.fast_channel,port=81),arm_motion=dict(speed_dps=60,acceleration_dps2=240,**self.arm_policy),
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

    def joystick_setup(self):
        self.page.locator('#manualLayout').select_option('joystick')
        self.page.wait_for_function("!document.getElementById('manualPad').hidden")

    def test_operations_navigation_full_viewport_and_positions(self):
        self.page.locator('#operationNav').click()
        self.page.wait_for_function("document.body.classList.contains('operation-mode')")
        self.assertFalse(any(p in ('/command','/control/acquire','/control/mode') for p,b in self.calls))
        for width,height,name in [(1440,900,'desktop'),(390,844,'mobile'),(844,390,'landscape'),(360,640,'small')]:
            self.page.set_viewport_size(dict(width=width,height=height))
            panel=self.page.locator('#operation-panel').bounding_box()
            self.assertEqual((panel['x'],panel['y'],panel['width'],panel['height']),(0,0,width,height))
            stick=self.page.locator('#driveJoystick').bounding_box()
            left=self.page.locator('#joyArmLeft').bounding_box()
            right=self.page.locator('#joyArmRight').bounding_box()
            self.assertLess(stick['x']+stick['width'],left['x'])
            self.assertLess(left['x'],right['x'])
            for selector in ['#driveJoystick','#joyArmLeft','#joyArmRight','#operationLaser','#operationStop','#operationBack']:
                b=self.page.locator(selector).bounding_box()
                self.assertGreaterEqual(b['x'],0);self.assertGreaterEqual(b['y'],0)
                self.assertLessEqual(b['x']+b['width'],width)
                self.assertLessEqual(b['y']+b['height'],height)
            self.assertLessEqual(self.page.locator('#operation-panel').evaluate('e=>e.scrollHeight'),height)
            self.page.screenshot(path=str(ROOT/f'build/operation-v018-{name}.png'))

    def test_operations_switch_stop_and_exit_clear_targets(self):
        self.joystick_setup()
        self.page.locator('#operationLaser').check()
        self.page.wait_for_function('controller.session===42&&!controller.pending')
        self.assertTrue(self.page.locator('#laser').is_checked())
        self.page.locator('#joyArmLeft').fill('130')
        self.page.locator('#joyArmRight').fill('100')
        self.page.wait_for_function('controller.desired.arm_left===130&&controller.desired.arm_right===100&&!controller.pending')
        self.page.locator('#operationStop').click()
        self.page.wait_for_function('controller.session===0')
        self.assertIn(('/stop',{}),self.calls)
        self.assertFalse(self.page.locator('#operationLaser').is_checked())
        self.page.locator('#operationLaser').check()
        self.page.wait_for_function('controller.session===42&&!controller.pending')
        self.stick_drag(0,-1)
        self.page.evaluate("location.hash='eyes-panel'")
        self.page.wait_for_function("document.getElementById('operation-panel').hidden")
        self.page.mouse.up()
        self.assertEqual(self.page.evaluate('[controller.session,controller.desired.left,controller.desired.right,controller.desired.laser]'),[0,0,0,False])
        self.assertTrue(any(p=='/control/release' for p,b in self.calls))

    def test_operations_direct_reload_does_not_acquire_or_replay(self):
        self.page.goto('http://robot/#operation-panel')
        self.page.wait_for_function("document.body.classList.contains('operation-mode')&&connected")
        self.page.reload()
        self.page.wait_for_function("document.body.classList.contains('operation-mode')&&connected")
        self.assertFalse(any(p in ('/command','/control/acquire','/control/mode') for p,b in self.calls))
        self.assertFalse(self.page.locator('#operationLaser').is_checked())
        self.page.locator('#operationBack').click()
        self.page.wait_for_function("document.getElementById('operation-panel').hidden")
        self.assertTrue(self.page.locator('.directions').is_visible())

    def test_operations_explicit_manual_power_and_busy_state(self):
        self.mode='auto';self.page.evaluate('refresh()')
        self.page.wait_for_function("mode==='auto'")
        self.joystick_setup()
        self.assertTrue(self.page.locator('#operationLaser').is_disabled())
        self.assertIn('Auto aktif',self.page.locator('#operationState').inner_text())
        self.assertFalse(any(p=='/control/mode' for p,b in self.calls))
        self.page.locator('#operationManual').click()
        self.page.wait_for_function("mode==='manual'&&!switchingMode")
        self.page.locator('#operationPower').fill('65')
        self.assertEqual(self.page.locator('#power').input_value(),'65')
        self.assertEqual(self.page.locator('#operationPowerValue').inner_text(),'65%')
        self.assertFalse(any(p=='/control/acquire' for p,b in self.calls))
        self.network_busy=True;self.page.evaluate('refresh()')
        self.page.wait_for_function('networkBusy')
        self.assertTrue(self.page.locator('#operationLaser').is_disabled())
        self.assertIn('Jaringan',self.page.locator('#operationState').inner_text())

    def test_operations_rapid_navigation_during_release_does_not_replay(self):
        self.joystick_setup()
        self.page.locator('#operationLaser').check()
        self.page.wait_for_function('controller.session===42&&!controller.pending')
        count=len([b for p,b in self.calls if p=='/control/acquire'])
        self.page.evaluate('''()=>{
          const original=controller.request;
          controller.request=async(path,body)=>{
            if(path==='/control/release')await new Promise(resolve=>window.finishRelease=resolve);
            return original(path,body);
          };
          location.hash='control-panel';
        }''')
        self.page.wait_for_function('operationTransition&&!!window.finishRelease')
        self.assertTrue(self.page.locator('#operationLaser').is_disabled())
        self.page.evaluate("location.hash='operation-panel'")
        self.page.wait_for_function("!operationTransition&&!document.getElementById('operation-panel').hidden")
        self.page.evaluate('finishRelease()')
        self.page.wait_for_timeout(150)
        self.assertTrue(self.page.locator('#operation-panel').is_visible())
        self.assertEqual(self.page.evaluate('[controller.session,controller.desired.left,controller.desired.right,controller.desired.laser]'),[0,0,0,False])
        self.assertEqual(count,len([b for p,b in self.calls if p=='/control/acquire']))

    def stick_drag(self,x=.5,y=-.5):
        self.page.locator('#driveJoystick').scroll_into_view_if_needed()
        box=self.page.locator('#driveJoystick').bounding_box()
        self.page.mouse.move(box['x']+box['width']/2,box['y']+box['height']/2)
        self.page.mouse.down()
        self.page.mouse.move(box['x']+box['width']*(.5+x*.5),box['y']+box['height']*(.5+y*.5))
        self.page.wait_for_function('controller.session===42&&!controller.pending')

    def test_laser_survives_direction_release_but_global_stop_clears(self):
        self.page.locator('#laser').check()
        self.page.wait_for_function('controller.session===42&&!controller.pending')
        self.page.locator('[data-drive="forward"]').dispatch_event('pointerdown',dict(pointerId=1))
        self.page.wait_for_function('controller.desired.left>0')
        self.page.locator('[data-drive="forward"]').dispatch_event('pointerup',dict(pointerId=1))
        self.page.wait_for_function('controller.desired.left===0&&controller.desired.right===0&&!controller.pending')
        self.page.wait_for_timeout(220)
        self.assertTrue(self.page.locator('#laser').is_checked())
        commands=[b for p,b in self.calls if p=='/command']
        self.assertTrue(commands[-1]['laser'])
        self.assertEqual((commands[-1]['left'],commands[-1]['right']),(0,0))
        self.assertFalse(any(p=='/control/release' for p,b in self.calls))
        self.page.locator('#stop').click()
        self.page.wait_for_function('controller.session===0')
        self.assertFalse(self.page.locator('#laser').is_checked())

    def test_joystick_mix_deadzone_and_power_limit(self):
        result=self.page.evaluate('''()=>{document.getElementById('power').value=80;return [[0,-1],[0,1],[-1,0],[1,0],[1,-1],[.01,.01]].map(p=>mixJoystick(...p));}''')
        for value,expected in zip(result[:5],[(.8,.8),(-.8,-.8),(-.8,.8),(.8,-.8),(.8,0)]):
            self.assertAlmostEqual(value['left'],expected[0])
            self.assertAlmostEqual(value['right'],expected[1])
        self.assertEqual((result[-1]['left'],result[-1]['right']),(0,0))

    def test_joystick_pointer_release_keeps_selected_laser(self):
        self.joystick_setup()
        self.page.locator('#operationLaser').check()
        self.page.wait_for_function('controller.session===42&&!controller.pending')
        self.stick_drag(.7,-.7)
        self.assertGreater(self.page.evaluate('controller.desired.left'),0)
        self.page.mouse.up()
        self.page.wait_for_timeout(250)
        self.assertEqual(self.page.evaluate('[controller.desired.left,controller.desired.right]'),[0,0])
        self.assertTrue(self.page.evaluate('controller.desired.laser&&controller.session===42'))
        self.assertFalse(any(p=='/control/release' for p,b in self.calls))

    def test_joystick_keyboard_stop_and_cancel(self):
        self.joystick_setup()
        stick=self.page.locator('#driveJoystick');stick.focus()
        self.page.keyboard.down('ArrowLeft')
        self.page.wait_for_function('controller.desired.left<0&&controller.desired.right>0')
        self.page.keyboard.up('ArrowLeft')
        self.page.wait_for_function('controller.session===0')
        self.page.locator('#operationLaser').check()
        self.page.wait_for_function('controller.session===42&&!controller.pending')
        self.stick_drag(0,-1)
        self.page.evaluate("releaseDrive(driveInput,true)")
        self.page.wait_for_function('controller.session===0')
        self.assertFalse(self.page.locator('#operationLaser').is_checked())
        self.page.mouse.up()

    def test_joystick_two_vertical_arms_sync_and_bounds(self):
        self.joystick_setup()
        self.page.locator('#joyArmLeft').fill('140')
        self.page.wait_for_function('controller.desired.arm_left===140&&!controller.pending')
        self.page.locator('#joyArmRight').fill('98')
        self.page.wait_for_function('controller.desired.arm_right===98&&!controller.pending')
        self.assertEqual(self.page.locator('#armLeftNumber').input_value(),'140')
        self.assertEqual(self.page.locator('#armRight').input_value(),'98')
        self.page.evaluate('''()=>{renderArmPolicy({min_left:20,max_left:150,min_right:40,max_right:120});}''')
        self.assertEqual(self.page.locator('#joyArmLeft').get_attribute('max'),'150')
        self.assertEqual(self.page.locator('#joyArmRight').get_attribute('min'),'40')
        self.assertEqual(self.page.locator('#joyArmLeft').evaluate("e=>getComputedStyle(e).writingMode"),'vertical-lr')
        self.assertTrue(all(b['left']==b['right']==0 for p,b in self.calls if p=='/command'))

    def test_joystick_layout_change_no_acquire_and_mobile(self):
        self.joystick_setup()
        self.assertFalse(self.page.locator('.directions').is_visible())
        self.assertFalse(any(p in ('/command','/control/acquire') for p,b in self.calls))
        self.page.set_viewport_size(dict(width=390,height=844))
        self.assertLessEqual(self.page.evaluate('document.documentElement.scrollWidth'),390)
        self.page.locator('#manualPad').screenshot(path=str(ROOT/'build/joystick-v017-mobile.png'))
        self.page.locator('#operationBack').click()
        self.page.wait_for_function("document.getElementById('operation-panel').hidden")
        self.assertTrue(self.page.locator('#manualPad').is_hidden())

    def test_joystick_auto_offline_blur_no_replay(self):
        self.joystick_setup();self.stick_drag(0,-1)
        self.page.evaluate("window.dispatchEvent(new Event('blur'))")
        self.page.wait_for_function('controller.session===0')
        self.page.mouse.up()
        self.page.evaluate("selectMode('auto')")
        self.page.wait_for_function("mode==='auto'")
        count=len([b for p,b in self.calls if p=='/command'])
        box=self.page.locator('#driveJoystick').bounding_box()
        self.page.mouse.click(box['x']+box['width']/2,box['y']+10)
        self.page.wait_for_timeout(150)
        self.assertEqual(count,len([b for p,b in self.calls if p=='/command']))

    def test_joystick_keyboard_repeat_cannot_restart_after_stop(self):
        self.joystick_setup()
        stick=self.page.locator('#driveJoystick');stick.focus()
        stick.dispatch_event('keydown',dict(key='ArrowUp',repeat=False))
        self.page.wait_for_function('controller.session===42&&!controller.pending')
        self.page.evaluate('controller.stop()')
        self.page.wait_for_function('controller.session===0')
        count=len([b for p,b in self.calls if p=='/command'])
        stick.dispatch_event('keydown',dict(key='ArrowUp',repeat=True))
        self.page.wait_for_timeout(150)
        self.assertEqual(count,len([b for p,b in self.calls if p=='/command']))

    def test_joystick_movement_and_arm_change_preserve_laser(self):
        self.joystick_setup()
        self.page.locator('#operationLaser').check()
        self.page.wait_for_function('controller.session===42&&!controller.pending')
        self.stick_drag(.5,-.8)
        self.page.evaluate("setArmTarget('armLeft',140)")
        self.page.wait_for_function('controller.desired.arm_left===140&&!controller.pending')
        self.page.mouse.up()
        self.page.wait_for_timeout(200)
        cmd=[b for p,b in self.calls if p=='/command'][-1]
        self.assertTrue(cmd['laser'])
        self.assertEqual((cmd['left'],cmd['right'],cmd['arm_left']),(0,0,140))

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
        self.assertTrue(self.page.locator("#wifi-settings").is_visible())
        self.assertFalse(self.page.locator('script[src],link[href^="http"],img[src^="http"]').count())
        self.assertNotIn("WebSockets", HTML)
        self.assertNotIn("ESP32-WROOM", HTML)

    def test_wifi_scan_select_save_password_not_displayed(self):
        self.page.locator('#wifiNav').click()
        self.page.locator('#scanWifi').click()
        self.page.wait_for_function("document.querySelectorAll('#wifiDetected option').length===3")
        self.page.locator('#wifiDetected').select_option(value='Backup WiFi')
        self.assertEqual(self.page.locator('#ssid').input_value(),'Backup WiFi')
        self.page.locator('#wifiPassword').fill('private-test-pass')
        self.page.locator('#saveWifi').click()
        self.page.wait_for_function("document.querySelector('#wifiProfiles').textContent.includes('Backup WiFi')")
        self.assertIn(('/settings',dict(ssid='Backup WiFi',password='private-test-pass')),self.calls)
        self.assertEqual(self.page.locator('#wifiPassword').input_value(),'')
        self.assertNotIn('private-test-pass',self.page.locator('#wifi-settings').inner_text())
        self.assertFalse(any(path in ('/command','/control/acquire','/stop') for path,_ in self.calls))
        self.assertFalse(self.page.locator('#wifi-settings img').count())

    def test_wifi_signal_connect_and_delete(self):
        self.wifi['profiles'].append(dict(ssid='Backup',priority=2,secured=True))
        self.wifi['networks']=[dict(ssid='Backup',rssi=-40,secured=True)]
        self.page.locator('#wifiNav').click()
        self.page.wait_for_function("document.querySelectorAll('#wifiProfiles button').length===4")
        self.assertIn('Backup',self.page.locator('#wifiProfiles .wifi-row').first.inner_text())
        self.assertNotIn('Naik',self.page.locator('#wifi-settings').inner_text())
        self.page.locator('#wifiProfiles').get_by_text('Sambung',exact=True).click()
        self.page.wait_for_function('!wifiBusy')
        self.assertIn(('/wifi/connect',dict(ssid='Backup')),self.calls)
        self.page.on('dialog',lambda dialog:dialog.accept())
        self.page.locator('#wifiProfiles').get_by_text('Hapus',exact=True).first.click()
        self.page.wait_for_function("!document.querySelector('#wifiProfiles').textContent.includes('Backup')")
        self.assertIn(('/wifi/delete',dict(ssid='Backup')),self.calls)

    def test_wifi_active_signal_visible_without_opening_settings(self):
        self.page.wait_for_function("document.querySelector('#wifiCurrent').textContent==='HuaweiJIN'")
        self.assertTrue(self.page.locator('#wifiCurrent').is_visible())
        self.assertIn('-55 dBm',self.page.locator('#wifiSignal').inner_text())
        self.assertEqual(self.page.locator('.wifi-bars .on').count(),4)
        self.assertTrue(self.page.get_by_role('button',name='Terhubung',exact=True).is_disabled())
        self.assertFalse(self.page.locator('#wifiAdd').evaluate('el=>el.open'))

    def test_wifi_disconnected_does_not_show_stale_active_ssid_or_signal(self):
        self.wifi.update(connected=False,state='connecting',current_ssid='stale',target_ssid='Backup',rssi=None)
        self.page.evaluate('loadWifi()')
        self.page.wait_for_function("document.querySelector('#wifiCurrent').textContent==='Hotspot robot'")
        self.assertIn('Mencoba Backup',self.page.locator('#wifiState').inner_text())
        self.assertNotIn('stale',self.page.locator('#wifiCurrent').inner_text())
        self.assertNotIn('dBm',self.page.locator('#wifiSignal').inner_text())

    def test_wifi_rejected_owner_does_not_global_stop(self):
        self.reject_wifi=True
        self.page.locator('#wifiNav').click()
        self.page.locator('#scanWifi').click()
        self.page.wait_for_function("document.querySelector('#wifiMessage').textContent.includes('release_control_or_wait_for_network')")
        self.assertFalse(any(path=='/stop' for path,_ in self.calls))

    def test_wifi_blank_password_requires_explicit_open_and_mobile_layout(self):
        self.page.set_viewport_size(dict(width=390,height=1100))
        self.page.locator('#wifiNav').click()
        self.page.locator('#wifiAdd summary').click()
        self.page.locator('#ssid').fill('Open WiFi')
        self.page.locator('#saveWifi').click()
        self.assertFalse(any(path=='/settings' and 'ssid' in body for path,body in self.calls))
        self.page.locator('#wifiOpen').check()
        self.page.locator('#saveWifi').click()
        self.page.wait_for_function('!wifiBusy')
        self.assertIn(('/settings',dict(ssid='Open WiFi',password='')),self.calls)
        self.assertTrue(self.page.evaluate('document.documentElement.scrollWidth<=innerWidth'))

    def demo_setup(self):
        self.page.wait_for_function('demoData!==null')
        for id,value in [('demoLength','120'),('demoWidth','90'),('demoRadius','10')]:
            self.page.locator('#'+id).fill(value)
        self.page.locator('#demoSave').click()
        self.page.wait_for_function('!demoBusy&&!demoDirty')

    def test_demo_simulation_has_no_actuator_lease_or_mode_change(self):
        self.demo_setup()
        self.page.locator('#demoSim').click()
        self.page.wait_for_function('demoData.running')
        self.assertIn(('/demo/start',dict(mode='simulation')),self.calls)
        self.assertFalse(any(p in ['/command','/control/acquire','/control/mode','/stop'] for p,_ in self.calls))
        self.assertIn('SIMULASI',self.page.locator('#demoBadge').inner_text())
        self.assertTrue(self.page.locator('#demoStart').is_disabled())

    def test_demo_pwm_percentages_roundtrip_without_actuation(self):
        self.demo_setup()
        self.page.locator('#demoAdvanced').evaluate('e=>e.open=true')
        self.page.locator('#demoPwm').fill('80')
        self.page.locator('#demoMinPwm').fill('60')
        self.page.locator('#demoDeadband').fill('50')
        self.page.locator('#demoSave').click()
        self.page.wait_for_function('!demoBusy&&!demoDirty')
        posted=[body for path,body in self.calls if path=='/demo/config'][-1]
        self.assertEqual((posted['pwm'],posted['min_pwm'],posted['deadband_pwm']),(.8,.6,.5))
        self.assertFalse(any(path in ['/command','/control/acquire'] for path,_ in self.calls))
        self.assertEqual(self.page.locator('#demoMinPwm').input_value(),'60')

    def test_demo_minimum_speed_block_is_explained(self):
        self.demo_setup()
        self.demo.update(minimum_speed_allowed=False,can_start_robot=False)
        self.page.evaluate('loadDemo()')
        self.page.wait_for_function('demoData.minimum_speed_allowed===false')
        self.assertTrue(self.page.locator('#demoStart').is_disabled())
        self.assertIn('kecepatan minimum motor melebihi',self.page.locator('#demoReadiness').inner_text())

    def test_demo_wizard_preparation_has_no_actuator_requests(self):
        self.demo_setup()
        self.page.locator('#demoPrepare').click()
        self.assertIn('Periksa persiapan',self.page.locator('#demoStepTitle').inner_text())
        self.page.once('dialog',lambda dialog:dialog.accept())
        self.page.locator('#demoHandsReady').click()
        self.page.wait_for_function('demoData.config.arms_confirmed')
        self.assertFalse(any(path in ['/command','/control/acquire','/control/mode','/demo/calibration/start'] for path,_ in self.calls))
        self.assertIn('Uji maju',self.page.locator('#demoStepTitle').inner_text())

    def test_demo_wizard_all_steps_and_explicit_start(self):
        self.demo_setup();self.demo['config']['arms_confirmed']=True
        self.page.locator('#demoPrepare').click();self.page.evaluate('loadDemo()')
        for kind in ['forward','cw','ccw']:
            self.page.wait_for_function('!document.querySelector("#demoWizardRun").hidden')
            before=len(self.calls)
            self.page.once('dialog',lambda dialog:dialog.accept());self.page.locator('#demoWizardRun').click()
            self.page.wait_for_function('demoData.calibrating')
            relevant=[path for path,_ in self.calls[before:] if path in ['/control/mode','/demo/zero','/demo/calibration/start']]
            self.assertEqual(relevant,['/control/mode','/demo/zero','/demo/calibration/start'])
            self.assertEqual([body for path,body in self.calls if path=='/demo/calibration/start'][-1],dict(kind=kind))
            self.demo.update(running=False,calibrating=False,sample_pending=True)
            self.page.evaluate('loadDemo()');self.page.wait_for_function('demoData.sample_pending')
            self.assertIn('cm' if kind=='forward' else 'derajat',self.page.locator('#demoResultLabel').inner_text())
            self.page.locator('#demoWizardMeasured').fill('4' if kind=='forward' else '45')
            self.page.locator('#demoWizardCoast').fill('0')
            if kind!='forward':self.page.locator('#demoDirection').check()
            self.page.locator('#demoWizardSave').click();self.page.wait_for_function('!demoData.sample_pending&&!demoBusy')
        self.assertFalse(any(path=='/demo/start' for path,_ in self.calls))
        self.page.once('dialog',lambda dialog:dialog.accept());self.page.locator('#demoWizardZero').click()
        self.page.wait_for_function('demoData.zero_valid')
        self.demo['can_start_robot']=True;self.page.evaluate('loadDemo()')
        self.page.wait_for_function('!document.querySelector("#demoWizardStart").disabled')
        self.page.once('dialog',lambda dialog:dialog.accept());self.page.locator('#demoWizardStart').click()
        self.page.wait_for_function('demoData.running')
        self.assertIn(('/demo/start',dict(mode='robot')),self.calls)

    def test_demo_wizard_pending_survives_reload_and_requires_direction(self):
        self.demo_setup();self.demo.update(sample_pending=True,sample_kind=1,sample_id=9,run_id=9)
        self.page.reload();self.page.wait_for_function('demoData&&demoData.sample_pending')
        self.assertTrue(self.page.locator('#demoResultBox').is_visible())
        self.assertFalse(any(path.startswith('/demo/') for path,_ in self.calls if path!='/demo/config'))
        self.page.locator('#demoWizardMeasured').fill('45');self.page.locator('#demoWizardCoast').fill('0')
        self.page.locator('#demoWizardSave').click()
        self.assertFalse(any(path=='/demo/calibration/result' for path,_ in self.calls))
        self.page.locator('#demoDirection').check();self.page.locator('#demoWizardSave').click()
        self.page.wait_for_function('!demoData.sample_pending')
        self.assertIn(('/demo/calibration/result',dict(sample_id=9,measured=45,coast_cm=0)),self.calls)

    def test_demo_wizard_rejected_measurement_no_calibration(self):
        for button in ['demoNoMovement','demoWrongDirection']:
            self.demo.update(sample_pending=True,sample_kind=1,sample_id=9,run_id=9)
            self.page.evaluate('loadDemo()');self.page.wait_for_function('demoData&&demoData.sample_pending')
            self.page.locator('#'+button).click();self.page.wait_for_function('!demoData.sample_pending')
        self.assertFalse(any(path=='/demo/calibration/result' for path,_ in self.calls))
        self.assertIn(('/demo/stop',dict(run_id=9)),self.calls)

    def test_demo_wizard_space_guard_and_mobile(self):
        self.demo_setup();self.demo['config']['arms_confirmed']=True;self.demo['calibration_forward_fits']=False
        self.page.set_viewport_size(dict(width=390,height=1100));self.page.locator('#demoPrepare').click();self.page.evaluate('loadDemo()')
        self.page.wait_for_function('demoData.calibration_forward_fits===false')
        self.assertTrue(self.page.locator('#demoWizardRun').is_disabled())
        self.assertIn('Ruang model belum cukup',self.page.locator('#demoStepInfo').inner_text())
        self.assertTrue(self.page.evaluate('document.documentElement.scrollWidth<=innerWidth'))
        self.assertTrue(self.page.locator('#demoGlobalStop').is_visible())

    def test_demo_zero_cancel_and_confirm_no_motion(self):
        self.demo_setup()
        self.page.locator('#demoAdvanced').evaluate('e=>e.open=true')
        self.page.once('dialog',lambda d:d.dismiss())
        self.page.locator('#demoZero').click()
        self.assertFalse(any(p=='/demo/zero' for p,_ in self.calls))
        self.page.once('dialog',lambda d:d.accept())
        self.page.locator('#demoZero').click()
        self.page.wait_for_function('demoData.zero_valid')
        self.assertIn(('/demo/zero',dict(confirmed_center=True,confirmed_fence=True)),self.calls)
        self.assertFalse(any(p=='/command' for p,_ in self.calls))

    def test_demo_real_start_requires_server_readiness_and_confirmation(self):
        self.demo_setup()
        self.page.locator('#demoAdvanced').evaluate('e=>e.open=true')
        self.assertTrue(self.page.locator('#demoStart').is_disabled())
        self.demo.update(can_start_robot=True,calibration_ready=True,zero_valid=True)
        self.page.evaluate('loadDemo()')
        self.page.wait_for_function('!document.querySelector("#demoStart").disabled')
        self.page.once('dialog',lambda d:d.accept())
        self.page.locator('#demoStart').click()
        self.page.wait_for_function('demoData.running')
        self.assertIn(('/demo/start',dict(mode='robot')),self.calls)

    def test_demo_scoped_stop_and_remonitor_after_reload(self):
        self.demo_setup()
        self.demo.update(running=True,run_id=51,phase='gesture',simulation=False)
        self.page.reload()
        self.page.wait_for_function('demoData&&demoData.running')
        self.assertIn('ROBOT NYATA',self.page.locator('#demoBadge').inner_text())
        self.page.locator('#demoStop').click()
        self.page.wait_for_function('!demoData.running')
        self.assertIn(('/demo/stop',dict(run_id=51)),self.calls)
        self.assertFalse(any(p=='/stop' for p,_ in self.calls))

    def test_demo_mobile_map_and_dirty_config_retained(self):
        self.page.set_viewport_size(dict(width=390,height=1100))
        self.page.wait_for_function('demoData!==null')
        self.page.locator('#demoLength').fill('155')
        self.page.evaluate('loadDemo()')
        self.page.wait_for_timeout(150)
        self.assertEqual(self.page.locator('#demoLength').input_value(),'155')
        self.assertTrue(self.page.evaluate('document.documentElement.scrollWidth<=innerWidth'))
        self.assertIn('Perkiraan:',self.page.locator('#demoPose').inner_text())

    def test_demo_calibration_is_explicit_and_uses_sample_id(self):
        self.demo_setup()
        self.mode='auto';self.demo.update(zero_valid=True);self.demo['config']['arms_confirmed']=True
        self.page.locator('#demoAdvanced').evaluate('e=>e.open=true')
        self.page.evaluate('refresh();loadDemo()')
        self.page.wait_for_function('!document.querySelector("[data-demo-cal=forward]").disabled')
        self.page.once('dialog',lambda d:d.accept())
        self.page.locator('[data-demo-cal=forward]').click()
        self.page.wait_for_function('demoData.sample_id===8')
        self.assertIn(('/demo/calibration/start',dict(kind='forward')),self.calls)
        self.demo.update(running=False,calibrating=False);self.page.evaluate('loadDemo()')
        self.page.wait_for_function('!document.querySelector("#demoCalSave").disabled')
        self.page.locator('#demoMeasured').fill('4.5')
        self.page.locator('#demoCalSave').click()
        self.page.wait_for_function('!demoBusy')
        self.assertIn(('/demo/calibration/result',dict(sample_id=8,measured=4.5,coast_cm=0)),self.calls)

    def test_network_busy_disables_actuators(self):
        self.network_busy=True
        self.page.evaluate('refresh()')
        self.page.wait_for_function('networkBusy')
        self.assertTrue(self.page.locator('#armLeft').is_disabled())
        self.assertTrue(self.page.locator('[data-drive=forward]').is_disabled())

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
        # Native mouse coordinates must not race the dashboard's smooth scroll.
        self.page.add_style_tag(content='html{scroll-behavior:auto!important}')
        button = self.page.locator('[data-drive="forward"]')
        button.scroll_into_view_if_needed()
        button.hover()
        self.page.mouse.down()
        self.page.wait_for_function('controller.desired.left>0&&controller.session===42&&!controller.pending')
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

    def test_exhibition_reset_no_motion_then_explicit_start_stop(self):
        self.page.on('dialog',lambda d:d.accept())
        self.page.wait_for_function('demoData!==null')
        self.assertTrue(self.page.locator('#exhibitStart').is_disabled())
        self.assertTrue(self.page.locator('#exhibitCenter').is_disabled())
        self.page.locator('#exhibitConfirmed').check()
        self.page.locator('#exhibitCenter').click()
        self.page.wait_for_function('demoData.exhibition.can_start')
        self.assertEqual(self.mode,'auto')
        self.assertTrue(any(p=='/demo/exhibition/center' and all(b.values()) for p,b in self.calls))
        self.assertFalse(any(p in ('/command','/control/acquire','/demo/exhibition/start') for p,b in self.calls))
        self.assertFalse(self.demo['config']['calibration'])
        self.page.locator('#exhibitStart').click()
        self.page.wait_for_function('demoData.running')
        self.assertIn(('/demo/exhibition/start',dict(mode='robot',accept_estimates=True)),self.calls)
        self.page.locator('#exhibitStop').click()
        self.page.wait_for_function('!demoData.running')
        self.assertTrue(self.page.locator('#exhibitStart').is_disabled())
        self.assertFalse(self.demo['exhibition']['center_valid'])

    def test_exhibition_reload_monitor_no_replay(self):
        self.demo.update(running=True,run_id=9,phase='gesture')
        self.demo['exhibition'].update(selected=True)
        self.page.reload()
        self.page.wait_for_function('demoData?.running===true')
        self.assertIn('Pameran ROBOT NYATA',self.page.locator('#exhibitStatus').inner_text())
        self.assertTrue(self.page.locator('#exhibitCenter').is_disabled())
        self.assertFalse(any(b is not None for p,b in self.calls))

    def test_exhibition_simulation_and_edit_invalidates_start(self):
        self.page.wait_for_function('demoData!==null')
        self.page.locator('#exhibitWidth').fill('40')
        self.page.locator('#exhibitSim').click()
        self.page.wait_for_function('demoData.running')
        self.assertIn(('/demo/exhibition/config',dict(width_cm=40,length_cm=30)),self.calls)
        self.assertIn(('/demo/exhibition/start',dict(mode='simulation',accept_estimates=True)),self.calls)
        self.assertFalse(any(p in ('/command','/control/acquire','/control/mode') for p,b in self.calls))

    def test_exhibition_mobile_and_foreign_owner(self):
        self.page.on('dialog',lambda d:d.accept())
        self.page.set_viewport_size(dict(width=390,height=844))
        self.session=42
        self.page.wait_for_function('demoData!==null')
        self.page.locator('#exhibitConfirmed').check()
        self.page.locator('#exhibitCenter').click()
        self.page.wait_for_function('!demoBusy')
        self.assertFalse(any(p=='/demo/exhibition/center' for p,b in self.calls))
        self.assertIn('Pengendali lain',self.page.locator('#demoMessage').inner_text())
        self.assertLessEqual(self.page.evaluate('document.documentElement.scrollWidth'),390)
        self.page.locator('#exhibitionPanel').screenshot(path=str(ROOT/'build/exhibition-v016-mobile.png'))


if __name__ == "__main__":
    (ROOT / "build").mkdir(exist_ok=True)
    unittest.main()
