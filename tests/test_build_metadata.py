import sys
import pathlib
import tempfile
import unittest
from datetime import datetime, timezone, timedelta
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / "scripts"))
from build_metadata import generate, verify, check_policy

class BuildIdentityTests(unittest.TestCase):
    def test_missing_rule_entrypoint_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            root = pathlib.Path(folder) / "repo"
            root.mkdir()
            (root / "AGENTS.md").write_text("Rules_Versioning.md")
            with self.assertRaises(RuntimeError): check_policy(root)
            (root / "AGENTS.md").write_text("Rules_Versioning.md Rules_Change_Notifications.md")
            check_policy(root)

    def test_regeneration_utc_and_stale_binary_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            sketch = pathlib.Path(folder)
            (sketch / "firmware_version.h").write_text('constexpr char kFirmwareVersion[] = "v0.3.4";')
            first = generate(sketch, datetime(2026, 10, 12, 8, 2, 3, tzinfo=timezone(timedelta(hours=7))))
            self.assertEqual(first["build_time_utc"], "2026-10-12T01:02:03Z")
            binary = sketch / "app.bin"
            binary.write_bytes(b'v0.3.4\0' + first["build_time_utc"].encode() + b'\0')
            self.assertIn("application_sha256", verify(binary, first))
            second = generate(sketch, datetime(2026, 10, 12, 1, 2, 4, tzinfo=timezone.utc))
            self.assertIn(second["build_time_utc"], (sketch / "firmware_build.h").read_text())
            with self.assertRaises(RuntimeError): verify(binary, second)
            with self.assertRaises(ValueError): generate(sketch, datetime(2026, 10, 12))

if __name__ == "__main__": unittest.main()
