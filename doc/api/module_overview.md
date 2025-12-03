# Module Overview

This document summarises the responsibilities of the main modules in the Motorcycle Display project.

## Entities (`include/logic/entities`)
- `VehicleData`: Holds normalised sensor values such as speed, RPM, throttle, ABS status, temperatures, and voltage.
- `AlertConfig`, `AlertState`: Describe alert rules, severities, and current activation state.

## Use Cases (`include/logic/use_cases`)
- `CanDataProcessor`: Polls the CAN bus, decodes raw frames, and produces validated `VehicleData` snapshots.
- `AlertEvaluator`: Applies configured alert rules to decoded vehicle data and reports changes to the alert port.
- `DataLogger`: Persists vehicle snapshots to CSV, adding headers automatically and ensuring directories exist.
- `OtaCoordinator`: Coordinates update availability checks and installations through the OTA port.
- `UserManualManager`: Loads the user manual content so it can be rendered on the dashboard.

## Ports (`include/logic/ports`)
- `ICanPort`, `IStoragePort`, `IClockPort`, `IOtaPort`, `IAlertPort`: Abstractions that the use cases depend on, enabling clean architecture boundaries.

## Controllers (`include/controller`)
- `DashboardController`: Central orchestrator tying together CAN polling, alert evaluation, logging, and view rendering.
- `AlertController`: Handles configuration of alert rules from UI or configuration sources.
- `OtaController`: Exposes OTA interactions to higher layers (e.g., UI menus).

## Drivers (`include/driver` / `src/driver`)
- `CanDriver`: Thread-safe CAN frame FIFO for ESP32 or simulator sources.
- `StorageDriver`: Filesystem-backed CSV persistence and manual loader.
- `ConsoleDisplayDriver`: Console-based renderer for development without hardware.
- `SimulatedTouchDriver`: Queue-based touch input simulator.
- `AlertNotifier`: Callback-driven alert sink for integration with UI or buzzer drivers.
- `OtaDriver`: Manifest-based OTA update checker and installer stub.
- `SystemClock`: Steady-clock backed monotonic timestamp source.

## UI (`include/ui`, `src/ui`)
- `DashboardView`: Bridges drivers and controllers, delivering render calls and bubbling up touch events.

## Executable (`src/main.cpp`)
- Sets up runtime wiring for simulators, registers default alert thresholds, and demonstrates CAN frame ingestion and rendering.

## Tooling (`tools/util/can_simulator.py`)
- Emits simulated CAN frames to a named pipe or console for development without a motorcycle connection.
