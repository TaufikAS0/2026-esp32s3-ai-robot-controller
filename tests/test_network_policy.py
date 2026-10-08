import pathlib
import sys
import unittest
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / "scripts"))
from network_policy import validate_defaults, validate_network_source, EXPECTED


class NetworkPolicyTests(unittest.TestCase):
    def source(self, **changes):
        values = {**EXPECTED, **changes}
        return "\n".join(f'constexpr char {key}[] = "{value}";' for key, value in values.items())

    def test_approved_profile(self):
        validate_defaults(self.source())

    def test_wrong_ssid_is_rejected(self):
        with self.assertRaises(ValueError):
            validate_defaults(self.source(stationSsid="HardwareTest"))

    def test_random_ap_password_is_rejected(self):
        with self.assertRaises(ValueError):
            validate_defaults(self.source(apPassword="random-device-password"))

    def test_wrong_station_password_is_rejected(self):
        with self.assertRaises(ValueError):
            validate_defaults(self.source(stationPassword="wrong"))

    def test_actual_network_source_uses_policy(self):
        source = (pathlib.Path(__file__).resolve().parents[1] / "02_Firmware/robot_controller/network_service.cpp").read_text()
        validate_network_source(source)

    def test_old_random_ap_implementation_is_rejected(self):
        with self.assertRaises(ValueError):
            validate_network_source('apPassword = secret("apPassword"); WiFi.begin(NetworkDefaults::stationSsid, NetworkDefaults::stationPassword);')

    def test_old_nvs_station_implementation_is_rejected(self):
        with self.assertRaises(ValueError):
            validate_network_source('apPassword = NetworkDefaults::apPassword; WiFi.begin(ssid.c_str(), password.c_str());')
