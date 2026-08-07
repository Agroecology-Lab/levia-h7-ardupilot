#!/usr/bin/env python3
import math
import statistics
import sys
import time

from pymavlink import mavutil


# Work around a pymavlink indexed-message issue in this repository version.
_original_add_message = mavutil.add_message


def _safe_add_message(messages, message_type, message):
    previous = messages.get(message_type)
    if previous is not None and hasattr(previous, "_instances") and previous._instances is None:
        previous._instances = {}
    return _original_add_message(messages, message_type, message)


mavutil.add_message = _safe_add_message

port = sys.argv[1] if len(sys.argv) > 1 else "/dev/cu.usbmodem101"
link = mavutil.mavlink_connection(port)
if link.wait_heartbeat(timeout=15) is None:
    raise RuntimeError("No heartbeat")

print(f"Connected to system {link.target_system}; capture started", flush=True)

# RAW_IMU, SCALED_IMU2/3, ATTITUDE and VIBRATION.
for message_id, period_us in ((27, 100000), (116, 100000), (129, 100000), (30, 100000), (241, 100000)):
    link.mav.command_long_send(
        link.target_system,
        link.target_component,
        mavutil.mavlink.MAV_CMD_SET_MESSAGE_INTERVAL,
        0,
        message_id,
        period_us,
        0,
        0,
        0,
        0,
        0,
    )

samples = {"RAW_IMU": [], "SCALED_IMU2": [], "SCALED_IMU3": []}
attitude = []
vibration = []
texts = []
start = time.monotonic()
next_notice = 10

while time.monotonic() - start < 40:
    elapsed = time.monotonic() - start
    if elapsed >= next_notice:
        print(f"MARK {next_notice}s", flush=True)
        next_notice += 10
    message = link.recv_match(blocking=True, timeout=0.5)
    if message is None:
        continue
    message_type = message.get_type()
    if message_type in samples:
        samples[message_type].append(
            (
                elapsed,
                int(message.xacc),
                int(message.yacc),
                int(message.zacc),
                int(message.xgyro),
                int(message.ygyro),
                int(message.zgyro),
            )
        )
    elif message_type == "ATTITUDE":
        attitude.append((elapsed, float(message.roll), float(message.pitch), float(message.yaw)))
    elif message_type == "VIBRATION":
        vibration.append(
            (
                elapsed,
                float(message.vibration_x),
                float(message.vibration_y),
                float(message.vibration_z),
                int(message.clipping_0),
                int(message.clipping_1),
                int(message.clipping_2),
            )
        )
    elif message_type == "STATUSTEXT":
        text = str(message.text)
        texts.append((elapsed, text))
        print(f"STATUS {elapsed:.1f}s {text}", flush=True)


def summarize(name, data):
    print(f"{name}: {len(data)} samples", flush=True)
    if len(data) < 2:
        return
    times = [row[0] for row in data]
    rates = [1.0 / (b - a) for a, b in zip(times, times[1:]) if b > a]
    accel_norm = [math.sqrt(row[1] ** 2 + row[2] ** 2 + row[3] ** 2) for row in data]
    gyro_norm = [math.sqrt(row[4] ** 2 + row[5] ** 2 + row[6] ** 2) for row in data]
    unchanged = sum(data[i][1:] == data[i - 1][1:] for i in range(1, len(data)))
    print(
        f"  rate median={statistics.median(rates):.1f}Hz "
        f"accel_norm={min(accel_norm):.0f}..{max(accel_norm):.0f}mg "
        f"gyro_norm_max={max(gyro_norm):.0f}mrad/s unchanged={unchanged}",
        flush=True,
    )
    for axis, index in (("ax", 1), ("ay", 2), ("az", 3), ("gx", 4), ("gy", 5), ("gz", 6)):
        values = [row[index] for row in data]
        print(f"  {axis}={min(values)}..{max(values)}", flush=True)


for key, value in samples.items():
    summarize(key, value)

if attitude:
    print(
        "ATTITUDE ranges rad: "
        f"roll={min(x[1] for x in attitude):.2f}..{max(x[1] for x in attitude):.2f} "
        f"pitch={min(x[2] for x in attitude):.2f}..{max(x[2] for x in attitude):.2f} "
        f"yaw={min(x[3] for x in attitude):.2f}..{max(x[3] for x in attitude):.2f}",
        flush=True,
    )

if vibration:
    print(
        "VIBRATION max: "
        f"x={max(x[1] for x in vibration):.2f} "
        f"y={max(x[2] for x in vibration):.2f} "
        f"z={max(x[3] for x in vibration):.2f}; "
        f"clipping final={vibration[-1][4:]}",
        flush=True,
    )

print(f"Status messages captured: {len(texts)}", flush=True)
