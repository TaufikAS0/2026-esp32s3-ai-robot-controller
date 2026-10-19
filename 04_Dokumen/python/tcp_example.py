"""Read-only TCP discovery. Never acquires a session or emits actuator targets."""
import argparse
import json
from robot_tcp_client import RobotTcpClient

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('host', help='Robot LAN IP or hostname')
    parser.add_argument('--port', type=int, default=9000)
    args = parser.parse_args()
    with RobotTcpClient(args.host, args.port) as robot:
        print(json.dumps(robot.status(), indent=2))
