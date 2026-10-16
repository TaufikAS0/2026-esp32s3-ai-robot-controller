"""Host-only safety tests. Never contacts a robot."""
import argparse
import copy
import json
from pathlib import Path
import sys
import tempfile
import time
import unittest
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from soak import Runner, SafetyError, DeviceLock, zero_wheels, verify_identity


def status():
    return dict(firmware_version='vtest', build_time_utc='test-build', sta_mac='aa:bb',
                hardware=dict(chip_model='ESP32-S3',flash_size_bytes=16777216,psram_size_bytes=8388608),
                access_mode='open_lan',owner='none',session=0,mode='manual',maintenance=False,uptime_ms=1000,
                commanded_output=dict(left=0,right=0,arm_left=0,arm_right=71,laser=False,servo_enabled=True),
                pwm=dict(left1_duty=0,left2_duty=0,right1_duty=0,right2_duty=0),
                eyes=dict(enabled=True,automatic=True,blink=True,swap=True,expression='happy',brightness=160,
                          tempo=1,play_mode='reactive',category='all',energy=2,rotate_left=True,rotate_right=True))


class SoakTests(unittest.TestCase):
    def test_zero_guard(self):
        b=dict(left=0,right=0,arm_left=0,arm_right=71,laser=False)
        zero_wheels(b)
        for key,value in [('left',.001),('right',-.1),('left',False),('arm_left',float('nan')),('laser',1)]:
            with self.assertRaises(SafetyError): zero_wheels({**b,key:value})
        with self.assertRaises(SafetyError): zero_wheels({})

    def test_identity(self):
        verify_identity(status(),'AA:BB','vtest','test-build')
        for key,value in [('sta_mac','wrong'),('firmware_version','wrong'),('build_time_utc','wrong')]:
            with self.assertRaises(SafetyError): verify_identity({**status(),key:value},'aa:bb','vtest','test-build')

    def test_singleton(self):
        with DeviceLock('test-device'):
            with self.assertRaises(SafetyError):
                with DeviceLock('test-device'): pass
        with DeviceLock('test-device'): pass

    def make_runner(self, root):
        return Runner(argparse.Namespace(output=Path(root)/'run',url='http://never-contact',seed=1,
                      hours=.001,scenario_seconds=1,mac='aa:bb',version='vtest',build='test-build',accepted_pose=None))

    def test_expired_producer_never_sends(self):
        with tempfile.TemporaryDirectory() as root:
            r=self.make_runner(root)
            r.session=42;r.last_ack=time.monotonic()-.31
            with patch.object(r,'request') as tx:
                with self.assertRaises(TimeoutError): r.command(0,71,True)
                tx.assert_not_called()
            r.pool.shutdown();r.log.close();r.csvfile.close()

    def test_transmission_boundary_guard(self):
        with tempfile.TemporaryDirectory() as root:
            r=self.make_runner(root)
            with patch('soak.urllib.request.build_opener') as network:
                with self.assertRaises(SafetyError):
                    r.request('/command',dict(left=.01,right=0,arm_left=0,arm_right=71,laser=False))
                network.assert_not_called()
            r.pool.shutdown();r.log.close();r.csvfile.close()

    def test_conflicting_owner_cannot_be_stopped(self):
        with tempfile.TemporaryDirectory() as root:
            r=self.make_runner(root)
            s=status();s['owner']='manual';s['session']=12
            with patch.object(r,'request',return_value=s) as tx:
                self.assertEqual(r.run(),1)
                self.assertTrue(all(call.args[0]=='/status' for call in tx.call_args_list))
            self.assertEqual(json.loads((r.out/'checkpoint.json').read_text())['state'],'failed')

    def test_maintenance_reboot_and_wheels(self):
        for mutation in ({'maintenance':True},{'commanded_output':{**status()['commanded_output'],'left':.1}}):
            with tempfile.TemporaryDirectory() as root:
                r=self.make_runner(root)
                with patch.object(r,'request',return_value={**status(),**mutation}):
                    with self.assertRaises(SafetyError): r.status()
                r.pool.shutdown();r.log.close();r.csvfile.close()
        with tempfile.TemporaryDirectory() as root:
            r=self.make_runner(root);r.last_uptime=2000
            with patch.object(r,'request',return_value=status()):
                with self.assertRaises(SafetyError): r.status()
            r.pool.shutdown();r.log.close();r.csvfile.close()

    def test_cancellation_and_release_scope(self):
        with tempfile.TemporaryDirectory() as root:
            r=self.make_runner(root);r.cancel.set()
            with self.assertRaises(InterruptedError): r.command(0,71,False)
            r.session=42
            with patch.object(r,'request') as tx:
                r.release()
                tx.assert_called_once_with('/control/release',{'session':42},timeout=1)
            r.pool.shutdown();r.log.close();r.csvfile.close()

    def test_failed_background_status_is_fatal(self):
        with tempfile.TemporaryDirectory() as root:
            r=self.make_runner(root)
            from concurrent.futures import Future
            f=Future();f.set_exception(SafetyError('different owner'))
            r.pending=[f]
            with self.assertRaises(SafetyError): r.drain()
            r.pool.shutdown();r.log.close();r.csvfile.close()

    def test_full_matrix_and_final_off_with_fake_device(self):
        with tempfile.TemporaryDirectory() as root:
            r=self.make_runner(root)
            r.deadline=time.monotonic()+11
            s=status();last_command=[None];commands=[]
            def fake(path,body=None,timeout=.3):
                if path=='/status':
                    s['uptime_ms']+=1
                    if last_command[0] and time.monotonic()-last_command[0]>.5:
                        s['owner']='none';s['session']=0;s['commanded_output']['laser']=False
                    return copy.deepcopy(s)
                if path=='/control/mode': s['mode']=body['mode']
                if path=='/control/acquire':
                    s['owner']='program';s['session']=42
                    return {'session':42,'timeout_ms':500}
                if path=='/command':
                    zero_wheels(body);commands.append(body.copy());last_command[0]=time.monotonic()
                    s['commanded_output'].update({k:body[k] for k in ('left','right','arm_left','arm_right','laser')})
                if path=='/control/release':
                    s['session']=0;s['owner']='none';s['commanded_output']['laser']=False
                return {'ok':True}
            with patch.object(r,'request',side_effect=fake),patch('soak.socket.create_connection'):
                self.assertEqual(r.run(),0)
            self.assertTrue(any(c['laser'] for c in commands))
            self.assertEqual(s['owner'],'none');self.assertFalse(s['commanded_output']['laser'])
            events=[json.loads(line) for line in (r.out/'events.jsonl').read_text().splitlines()]
            names={e['scenario'] for e in events if e['event']=='scenario_start'}
            from soak import SCENARIOS
            self.assertEqual(names,set(SCENARIOS))


if __name__ == '__main__': unittest.main()
