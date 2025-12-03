# Motorcycle Dashboard Display

An ESP32-powered, touch-enabled motorcycle dashboard that decodes CAN bus telemetry, renders live gauges, logs structured data, supports OTA firmware updates, configurable alerts, and an onboard user manual.

## Features
- Real-time decoding of speed, RPM, throttle, ABS status, engine temperature, and battery voltage from the CAN bus.
- Touch-driven dashboard UI with quick access to alerts and a built-in user manual.
- Configurable alert engine with severity levels and CSV-backed logging for diagnostics.
- OTA update workflow driven by manifest files for rapid feature delivery.
- Python-based CAN simulator for development without a live motorcycle connection.
- Digital twin harness with an in-memory display driver for deterministic end-to-end tests on the host PC.
## Hardware Requirements
- ESP32 module with dual-core support and Wi-Fi connectivity (e.g., ESP32-WROVER).
- 5" capacitive touch display (SPI or RGB interface) compatible with ESP32.
- CAN transceiver (e.g., SN65HVD230) wired to ESP32 CAN pins.
- microSD or SPI flash partition for persistent logs and manuals.

## Software Stack
- Language: C++20 (ESP-IDF or desktop simulation build via CMake).
- Build system: CMake 3.20+.
- Optional host tools: Python 3.10+, `python-can`, `rich` (see `requirements.txt`).

## Repository Layout
- `src/` and `include/`: Clean Architecture layers (entities, use cases, controllers, drivers, UI, simulation digital twin).
- `data/`: Runtime assets such as manuals and OTA manifests.
- `doc/`: Comprehensive documentation (design diagrams, user manual, test plan, API notes).
- `tests/`: Unit tests for core business logic.
- `tools/util/can_simulator.py`: Host-side CAN traffic generator.

## Setup Instructions
1. **Clone and configure ESP-IDF (hardware build)**
	- Follow the ESP-IDF getting started guide for Windows PowerShell.
	- Ensure `idf.py` and the Xtensa toolchain are available in your `PATH`.
2. **Install host dependencies**
	```powershell
	python -m venv .venv
	.\.venv\Scripts\Activate.ps1
	pip install -r requirements.txt
	```
3. **Configure CMake build (simulation on host PC)**
	```powershell
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build
	```
4. **Configure ESP-IDF build (firmware)**
	- Adapt `src/CMakeLists.txt` to the ESP-IDF component model or wrap it in an ESP-IDF component as required.
	- Use `idf.py menuconfig` to set CAN, display, storage, and OTA parameters.

## Usage Examples
- **Run desktop simulator**
  ```powershell
  cmake --build build --target motorcycle_dashboard
  .\build\motorcycle_dashboard.exe
  ```
- **Simulate CAN traffic**
  ```powershell
  python tools/util/can_simulator.py --duration 120 --interval 0.5 --csv data/logs/simulator_log.csv
  ```
- **View logs**: Inspect `data/logs/vehicle_log.csv` for live data or the simulator log for synthetic data.
- **Display manual**: Swipe right on the dashboard (or inspect `data/manual/dashboard_manual.md`).

## Digital Twin and Simulator Workflow

- **Run the digital twin integration test**
	```powershell
	cmake --build build --target motorcycle_tests
	ctest --test-dir build --tests-regex motorcycle_tests --output-on-failure
	```
	The `DashboardDigitalTwinTests` feed synthetic CAN frames into the `simulation::DashboardDigitalTwin`, exercising alert evaluation, CSV logging, and the `DigitalDisplayDriver` snapshot.
- **Experiment interactively**: Instantiate `simulation::DashboardDigitalTwin` in a host application, enqueue frames with `simulation::stMakeFrame`, and inspect `DigitalDisplayDriver::stLastData()` or `vLastAlerts()` for visual verification without hardware.
- **Simulate CAN on the command line**: Combine the Python CAN simulator with the digital twin to prototype complex ride scenarios before deploying to the ESP32.

## Testing Instructions
1. Configure the build directory with tests enabled (default).
2. Build and run tests (unit and digital twin integration):
	```powershell
	cmake --build build --target motorcycle_tests
	ctest --test-dir build
	```
3. Review coverage by integrating your preferred coverage tool (e.g., `gcovr` for GCC builds).

## Logging & Error Handling
- Runtime logs are appended to CSV files to support ingestion by analytics tools.
- `DataLogger` writes headers once and records:

  | Column             | Description                        |
  | ------------------ | ---------------------------------- |
  | `timestamp_ms`     | Monotonic clock in milliseconds    |
  | `speed_kph`        | Vehicle speed                      |
  | `rpm`              | Engine revolutions per minute      |
  | `throttle_percent` | Throttle opening (0-100%)          |
  | `abs_active`       | Binary ABS state                   |
  | `engine_temp_c`    | Engine coolant temperature         |
  | `battery_voltage`  | Electrical system voltage (V)      |

- Errors in storage or OTA flows return `false`/`std::nullopt`, enabling the caller to surface meaningful messages without leaking sensitive information.

## Security Considerations
- No secrets are stored in source control; configure Wi-Fi credentials via secure ESP-IDF partition or environment variables.
- Input validation occurs in CAN decoding and alert configuration to prevent invalid states.
- OTA downloads must be served over TLS in production; integrate signature verification before deployment.
- Avoid logging sensitive rider information; CSV logs focus on vehicle telemetry only.

## Documentation
- **User manual**: `doc/user/dashboard_manual.md`
- **Architecture & diagrams**: `doc/design/diagrams/architecture.md`
- **Module overview**: `doc/api/module_overview.md`
- **Test plan**: `doc/complience/test_plan.md`

## Contribution Guidelines
1. Fork the repository and create a feature branch.
2. Follow Clean Code principles—small, intention-revealing functions; avoid duplication; document intent when non-obvious.
3. Maintain Clean Architecture boundaries (use cases depend on ports, not concrete drivers).
4. Add or update unit tests under `tests/` ensuring >80% coverage on business logic.
5. Run `ctest` and the CAN simulator (if applicable) before submitting a pull request.
6. Update documentation and diagrams when behaviour or architecture changes.
7. All pull requests must pass the CI workflow (`Code Review Checks` and `Build and Test`) before merge.

## Next Steps
- Integrate hardware-specific CAN, display, and touch drivers using ESP-IDF drivers or vendor libraries.
- Extend OTA workflow with signed manifests and delta updates.
- Build richer UI components (themes, trip history) leveraging the existing Clean Architecture foundation.
