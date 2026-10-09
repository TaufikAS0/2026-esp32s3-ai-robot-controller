"""Build gate for the approved lab profile; cannot prove an AI read prose."""
from pathlib import Path
import re

EXPECTED = {"stationSsid": "HuaweiJIN", "stationPassword": "jayaabadi100", "apPassword": "12345678"}


def validate_defaults(source):
    for key, expected in EXPECTED.items():
        values = re.findall(r'constexpr\s+char\s+' + key + r'\[\]\s*=\s*"([^"\\]*)"\s*;', source)
        if values != [expected]:
            raise ValueError(f"Lab network policy mismatch: {key}; read Rules_Kredensial_WiFi / WiFi_HuaweiJIN")


def validate_network_source(source):
    if not re.search(r'apPassword\s*=\s*NetworkDefaults::apPassword\s*;', source):
        raise ValueError("AP password must use the approved lab default")
    if re.search(r'secret\(\s*"apPassword"', source):
        raise ValueError("Random AP passwords contradict the mandatory lab policy")
    calls = re.findall(r'WiFi\.begin\s*\(([^;]*)\)\s*;', source)
    if [re.sub(r'\s+', '', call) for call in calls] != ["NetworkDefaults::stationSsid,NetworkDefaults::stationPassword"]:
        raise ValueError("STA connection must use the approved lab profile")


def check(root):
    validate_defaults((root / "02_Firmware/robot_controller/network_defaults.h").read_text(encoding="utf-8"))
    validate_network_source((root / "02_Firmware/robot_controller/network_service.cpp").read_text(encoding="utf-8"))
    agents = (root / "AGENTS.md").read_text(encoding="utf-8")
    for required in ("Rules_Kredensial_WiFi.md", "WiFi_HuaweiJIN.md"):
        if required not in agents:
            raise ValueError(f"Mandatory agent rule entrypoint missing: {required}")
    # Standalone GitHub builds use this approved gate; local builds also read vault.
    vault = next((root.parent / name for name in ("2026 Vault Firmware Rules", "2026-vault-firmware-rules")
                  if (root.parent / name).exists()), None)
    if vault is not None:
        rules = (vault / "01_Rules/Rules_Kredensial_WiFi.md").read_text(encoding="utf-8")
        profile = (vault / "04_Profiles/WiFi/WiFi_HuaweiJIN.md").read_text(encoding="utf-8")
        if "WiFi_HuaweiJIN" not in rules or "12345678" not in rules:
            raise ValueError("Obsidian credential rule does not match the approved lab policy")
        for key, expected in (("ssid", EXPECTED["stationSsid"]), ("password", EXPECTED["stationPassword"])):
            if not re.search(r'^' + key + r':\s*' + re.escape(expected) + r'\s*$', profile, re.M):
                raise ValueError(f"Obsidian lab profile mismatch: {key}")
        print("Read Obsidian: Rules_Kredensial_WiFi.md and WiFi_HuaweiJIN.md")
    print("Lab network policy PASS: STA HuaweiJIN; AP password 12345678")


if __name__ == "__main__":
    check(Path(__file__).resolve().parents[1])
