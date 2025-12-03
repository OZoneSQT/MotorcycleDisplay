#!/usr/bin/env python3
"""CAN data simulator for development without motorcycle hardware."""

from __future__ import annotations

import argparse
import csv
import random
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable, Iterator, List, Optional, TYPE_CHECKING

try:
    import can  # type: ignore[import-not-found, import-untyped]
except ImportError:  # pragma: no cover - optional dependency
    can = None

if TYPE_CHECKING:
    from can import BusABC as CanBusABC, Message as CanMessage
else:
    CanBusABC = Any
    CanMessage = Any

BusABC = CanBusABC
Message = CanMessage

try:
    from rich.progress import track
except ImportError:  # pragma: no cover - optional dependency
    track = None


CAN_IDS = {
    "speed": 0x100,
    "rpm": 0x101,
    "throttle": 0x102,
    "abs": 0x103,
    "engine_temp": 0x104,
    "battery": 0x105,
}

@dataclass
class SimulatedSample:
    timestamp: float
    speed: int
    rpm: int
    throttle: int
    abs_active: int
    engine_temp: int
    battery_voltage: int

    def to_frames(self) -> List[Message]:
        frames: List[Message] = []
        if can is None:
            return frames
        # Build python-can Message frames (requires python-can installed)
        frames.append(
            can.Message(arbitration_id=CAN_IDS["speed"], data=[self.speed & 0xFF], is_extended_id=False)
        )
        frames.append(
            can.Message(
                arbitration_id=CAN_IDS["rpm"],
                data=[self.rpm & 0xFF, (self.rpm >> 8) & 0xFF],
                is_extended_id=False,
            )
        )
        frames.append(can.Message(arbitration_id=CAN_IDS["throttle"], data=[self.throttle & 0xFF], is_extended_id=False))
        frames.append(can.Message(arbitration_id=CAN_IDS["abs"], data=[self.abs_active & 0x01], is_extended_id=False))
        frames.append(can.Message(arbitration_id=CAN_IDS["engine_temp"], data=[self.engine_temp & 0xFF], is_extended_id=False))
        frames.append(can.Message(arbitration_id=CAN_IDS["battery"], data=[self.battery_voltage & 0xFF], is_extended_id=False))
        return frames

    def to_row(self) -> List[str]:
        return [
            f"{int(self.timestamp * 1000)}",
            f"{self.speed}",
            f"{self.rpm}",
            f"{self.throttle}",
            f"{self.abs_active}",
            f"{self.engine_temp}",
            f"{self.battery_voltage / 10.0:.1f}",
        ]


def generate_samples(count: int) -> Iterator[SimulatedSample]:
    for _ in range(count):
        yield SimulatedSample(
            timestamp=time.time(),
            speed=random.randint(0, 180),
            rpm=random.randint(800, 9000),
            throttle=random.randint(0, 100),
            abs_active=random.randint(0, 1),
            engine_temp=random.randint(60, 120),
            battery_voltage=random.randint(120, 150),
        )

def write_csv(path: Path, rows: Iterable[List[str]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    new_file = not path.exists()
    with path.open("a", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        if new_file:
            writer.writerow(
                ["timestamp_ms", "speed_kmh", "rpm", "throttle_pct", "abs_active", "engine_temp_c", "battery_v"]
            )
        for row in rows:
            writer.writerow(row)

def send_frames(bus: Optional[BusABC], frames: Iterable[Message], sleep_s: float) -> None:
    if bus is None:
        return
    for frame in frames:
        try:
            bus.send(frame, timeout=0.1)
        except Exception:
            # Ignore send errors in the simulator
            pass
        if sleep_s > 0:
            time.sleep(sleep_s)

def run(duration: float, interval: float, csv_path: Path, use_bus: bool) -> None:
    iterations = int(duration / interval) if duration > 0 else 0
    iterator = track(range(iterations), description="Simulating") if track and iterations > 0 else range(iterations)

    bus: Optional[BusABC] = None
    if use_bus and can is not None:
        try:
            bus = can.Bus()
        except Exception:
            print("Warning: failed to open CAN bus; proceeding without bus")

    for _ in iterator:
        sample = next(generate_samples(1))
        write_csv(csv_path, [sample.to_row()])
        if bus is not None:
            send_frames(bus, sample.to_frames(), interval / 6 if interval > 0 else 0)
        else:
            print(
                f"{int(sample.timestamp * 1000)} | speed={sample.speed} rpm={sample.rpm} throttle={sample.throttle} temp={sample.engine_temp}"
            )
        time.sleep(interval)


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Simulate CAN data for the motorcycle dashboard")
    parser.add_argument("--duration", type=float, default=30.0, help="Simulation duration in seconds.")
    parser.add_argument("--interval", type=float, default=1.0, help="Interval between frames in seconds.")
    parser.add_argument(
        "--csv", type=Path, default=Path("data/logs/simulator_log.csv"), help="Path to append CSV samples."
    )
    parser.add_argument("--bus", action="store_true", help="Send frames on python-can virtual bus if available.")
    return parser.parse_args()


def main() -> None:
    args = parse_arguments()
    if args.duration <= 0 or args.interval <= 0:
        raise ValueError("duration and interval must be positive numbers")
    run(args.duration, args.interval, args.csv, args.bus)


if __name__ == "__main__":
    main()
