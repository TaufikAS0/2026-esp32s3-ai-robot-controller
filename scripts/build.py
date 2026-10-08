"""Portable, pinned Arduino CLI build with OTA image-size/partition verification."""
import argparse
import csv
import pathlib
import shutil
import subprocess
from network_policy import check as check_network_policy

ROOT = pathlib.Path(__file__).resolve().parents[1]
CORE = "3.3.10"
FQBN = "esp32:esp32:esp32s3:FlashSize=16M,PSRAM=opi,PartitionScheme=app3M_fat9M_16MB,CDCOnBoot=default,USBMode=hwcdc"
SKETCH = ROOT / "02_Firmware" / "robot_controller"
BUILD = ROOT / "build"


def check_artifacts():
    slots = []
    with (SKETCH / "partitions.csv").open() as source:
        for row in csv.reader(line for line in source if not line.startswith("#")):
            if len(row) >= 5 and row[1] == "app":
                slots.append((int(row[3], 0), int(row[4], 0)))
    assert len(slots) == 2 and slots[0][1] == slots[1][1]
    assert slots[0][0] + slots[0][1] <= slots[1][0]
    assert slots[1][0] + slots[1][1] <= 16 * 1024 * 1024
    binary = BUILD / "robot_controller.ino.bin"
    size = binary.stat().st_size
    if size > min(s[1] for s in slots):
        raise RuntimeError("Application binary exceeds OTA slot")
    # Compare actual emitted partition table (32-byte entries), not just source CSV.
    import struct
    partition_bytes = (BUILD / "robot_controller.ino.partitions.bin").read_bytes()
    actual = []
    for offset in range(0, len(partition_bytes) - 31, 32):
        magic, kind, subtype, address, length = struct.unpack_from("<HBBII", partition_bytes, offset)
        if magic == 0x50AA and kind == 0:
            actual.append((address, length))
    if actual != slots:
        raise RuntimeError(f"Emitted OTA partitions differ: {actual} != {slots}")
    print(f"OTA application: {size:,} bytes; each slot: {slots[0][1]:,} bytes; emitted partitions verified")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--cli", default=shutil.which("arduino-cli"))
    args = parser.parse_args()
    check_network_policy(ROOT)
    if not args.cli:
        parser.error("arduino-cli not found; supply --cli /path/to/arduino-cli")
    installed = subprocess.check_output([args.cli, "core", "list", "--format", "json"], text=True)
    import json
    data = json.loads(installed)
    platforms = data if isinstance(data, list) else data.get("platforms", [])
    if not any(p.get("id") == "esp32:esp32" and p.get("installed_version") == CORE for p in platforms):
        raise SystemExit(f"Install pinned core: arduino-cli core install esp32:esp32@{CORE}")
    BUILD.mkdir(exist_ok=True)
    subprocess.run([args.cli, "compile", "--fqbn", FQBN,
                    "--build-property", "upload.maximum_size=6291456",
                    "--output-dir", str(BUILD), str(SKETCH)], check=True)
    check_artifacts()
