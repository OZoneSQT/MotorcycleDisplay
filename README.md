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
- Language: C++23 (ESP-IDF or desktop simulation build via CMake).
- Build system: CMake 3.20+.
- Optional host tools: Python 3.11, `python-can`, `rich`, and a C++23 toolchain (Visual Studio Build Tools or LLVM clang).

## Repository Layout
- `Implementation/`: Build-ready source tree.
  - `src/` and `include/`: Clean Architecture layers (entities, use cases, controllers, drivers, UI, simulation digital twin).
  - `tests/`: Unit and integration coverage for core business logic and the digital twin.
  - `tools/`: CI utilities, build helpers, deployment scripts, and simulators (e.g., `tools/util/can_simulator.py`).
  - `data/`: Runtime assets such as manuals, CSV logs, and OTA manifests.
  - `design/`: Lightweight developer notes that complement the main documentation shard.
- `Documentation/`: Comprehensive references (design diagrams, compliance records, user manuals, API notes).
- `Commercial/` & `Maintenance/`: Business and support collateral retained alongside the source tree.

## Setup Instructions
1. **Run the provisioning script (PowerShell 7 required)**
	```powershell
	.\Implementation\Run_Setup.cmd -Configure -BuildType Debug
	```
	The script bootstraps Python 3.11, CMake, Ninja, and a virtual environment at `Implementation\.venv`. If no C++ compiler is detected, it will prompt you to install Visual Studio Build Tools (C++ workload) or LLVM clang before continuing.
	Common compiler options:
	- Visual Studio Build Tools: `winget install --id Microsoft.VisualStudio.2022.BuildTools --override "--quiet --add Microsoft.VisualStudio.Workload.VCTools" --scope user`
	- LLVM clang toolchain: `winget install --id LLVM.LLVM --scope user`
2. **Activate the virtual environment when developing**
	```powershell
	.\Implementation\.venv\Scripts\Activate.ps1
	```
3. **Build and test (Windows/MSVC)**
	Use the setup script in the same PowerShell 7 session before invoking CMake so the Visual Studio developer environment is loaded. After the initial provisioning you can skip virtual-environment work with `-SkipVenv`.
	```powershell
	# From the repository root inside PowerShell 7
	.\Implementation\Setup.ps1 -SkipVenv
	cmake --build build --target motorcycle_dashboard
	ctest --test-dir build --output-on-failure
	```
	If you prefer LLVM clang, add `-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++` to the initial configure command and run future builds inside the same PowerShell session.
4. **Configure ESP-IDF (hardware firmware)**
	- Mirror the `Implementation/src` layout inside an ESP-IDF component or extend the existing CMake files.
	- Use `idf.py menuconfig` to set CAN, display, storage, and OTA parameters, ensuring secrets remain outside source control.

## Usage Examples
- **Run desktop simulator**
  ```powershell
	.\Implementation\Setup.ps1 -SkipVenv
	cmake --build build --target motorcycle_dashboard
	.\build\motorcycle_dashboard.exe
  ```
- **Simulate CAN traffic**
  ```powershell
	python Implementation/tools/util/can_simulator.py --duration 120 --interval 0.5 --csv Implementation/data/logs/simulator_log.csv
  ```
- **View logs**: Inspect `Implementation/data/logs/vehicle_log.csv` for live data or the simulator log for synthetic data.
- **Display manual**: Swipe right on the dashboard (or inspect `Implementation/data/manual/dashboard_manual.md`).

## Digital Twin and Simulator Workflow

- **Run the digital twin integration test**
 	```powershell
	.\Implementation\Setup.ps1 -SkipVenv
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
	.\Implementation\Setup.ps1 -SkipVenv
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
- **User manual**: `Documentation/user/dashboard_manual.md`
- **Architecture & diagrams**: `Documentation/design/diagrams/architecture.md`
- **Module overview**: `Documentation/api/module_overview.md`
- **Test plan**: `Documentation/complience/test_plan.md`

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
