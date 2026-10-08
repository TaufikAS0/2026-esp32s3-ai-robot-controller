"""Explicit demo; credentials supplied outside the repository."""
import os
import time
from robot_client import RobotClient

with RobotClient(os.environ["ROBOT_URL"], os.environ["ROBOT_TOKEN"]) as robot:
    print(robot.status())
    robot.acquire()
    # Keep producing fresh targets. A sleeping/stalled producer loses its lease.
    until = time.monotonic() + 1
    while time.monotonic() < until:
        robot.drive(0.2, 0.2)
        time.sleep(0.05)
    robot.stop()
