# Motorcycle Dashboard Display

An ESP32-powered, touch-enabled motorcycle dashboard that decodes CAN bus telemetry, renders live gauges, logs structured data, supports OTA firmware updates, configurable alerts, and an onboard user manual.

## Features
- Real-time decoding of speed, RPM, throttle, ABS status, engine temperature, and battery voltage from the CAN bus.
- Touch-driven dashboard UI with quick access to alerts and a menu-driven user manual (Home/Back navigation).
- Configurable alert engine with severity levels and CSV-backed logging for diagnostics.
- OTA update workflow driven by manifest files for rapid feature delivery.
- Python-based CAN simulator for development without a live motorcycle connection.
- Digital twin harness with an in-memory display driver for deterministic end-to-end tests on the host PC.
- Desktop interactive simulator with keyboard shortcuts and a control panel window for telemetry tuning, alert monitoring, and touch events.
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

## Application Metadata (.env)
`Implementation/.env` centralizes runtime metadata that the application prints at boot and records through the audit log. Add or override keys using `key=value` pairs:

```dotenv
# Optional; defaults are applied when keys are missing
version=1.0.0
build_id=build-2025-12-04
app_name=Motorcycle Dashboard
copyright=2025 OZoneSQT
contact_info=support@motorcycledashboard.example
ota_source_link=https://updates.motorcycle.example/ota_manifest.json
debug=debug_false  # use debug_true to enable verbose mode
```

Invalid or malformed entries fall back to defaults and are reported in `Implementation/data/logs/audit_log.csv`.

Run `python Implementation/tools/util/verify_env_sync.py` after editing metadata to confirm the compiled defaults in `AppMetadata.hpp` match the `.env` overrides. Pass `--update-header` to rewrite the header automatically when the `.env` file becomes the new source of truth.

## Usage Examples
- **Run desktop simulator**
  ```powershell
	.\Implementation\Setup.ps1 -SkipVenv
	cmake --build build --target motorcycle_dashboard
	.\build\motorcycle_dashboard.exe
  ```
  ```powershell
	.\Implementation\Setup.ps1 -SkipVenv
	cmake --build build --target motorcycle_twin_simulator
	.\build\motorcycle_twin_simulator.exe
  ```
  The simulator opens a dashboard window plus a `Twin Control Panel` window where you can edit telemetry fields, trigger touch events, and watch alerts update live. Toggle the panel with the `P` key if you close or hide it.
	Manual navigation example: press `1`-`9` to open the corresponding child topic, `B` to go back, and `H` to return to the manual home menu.
	Resize the control panel to reveal the CAN tools section, then enter a CAN identifier (hex or decimal) and up to eight hex bytes to enqueue custom frames into the twin.
- **Helper scripts**
  ```powershell
	# Full project build (dashboard, tests, simulator)
	pwsh Implementation\tools\util\build_project.ps1 -Configuration Release

	# Build-and-run helper for the interactive simulator (omit -NoBuild to rebuild first)
	pwsh Implementation\tools\util\run_twin_simulator.ps1 -Configuration Debug

	# Build (optional) and execute the full CTest suite
	pwsh Implementation\tools\util\run_tests.ps1 -Configuration Debug
  ```
- **Simulate CAN traffic**
  ```powershell
	python Implementation/tools/util/can_simulator.py --duration 120 --interval 0.5 --csv Implementation/data/logs/simulator_log.csv
  ```
- **View logs**: Inspect `Implementation/data/logs/vehicle_log.csv` for live data or the simulator log for synthetic data.
- **View logs**: Inspect `Implementation/data/logs/vehicle_log.csv` for live data or the simulator log for synthetic data.
- **Audit trail**: Review `Implementation/data/logs/audit_log.csv` for CAN command traceability and software error codes.
- **Display manual**: Swipe right on the dashboard (or inspect `Implementation/data/manual/dashboard_manual.menu`).
- **Tweak CAN IDs**: Edit `Implementation/bus.env` to override CAN identifiers (e.g., `CAN_ID_SPEED=0x200`). Leave values untouched to stick with the compiled defaults.

## Digital Twin and Simulator Workflow

- **Run the digital twin integration test**
 	```powershell
	.\Implementation\Setup.ps1 -SkipVenv
	cmake --build build --target motorcycle_tests
	ctest --test-dir build --tests-regex motorcycle_tests --output-on-failure
	```
	The `DashboardDigitalTwinTests` feed synthetic CAN frames into the `simulation::DashboardDigitalTwin`, exercising alert evaluation, CSV logging, and the `DigitalDisplayDriver` snapshot.
- **Experiment interactively**: Start `motorcycle_twin_simulator`, then use the control panel window (or keyboard shortcuts) to change telemetry, toggle ABS, enqueue touch taps, and observe alert/activity updates without hardware.
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
- `AuditLogger` captures every CAN command (enqueue and decode) alongside software error codes for compliance-ready traceability.

	| Column          | Description                                      |
	| --------------- | ------------------------------------------------ |
	| `timestamp_ms`  | Millisecond-resolution clock or frame timestamp  |
	| `event_type`    | `CAN_COMMAND` or `SOFTWARE_ERROR`                |
	| `source`        | Module responsible for the event                 |
	| `identifier`    | CAN ID (hex) or software error code              |
	| `payload`       | Space-delimited CAN payload bytes (if available) |
	| `message`       | Contextual message (e.g., DLC, failure reason)   |
- Application metadata from `.env` is parsed at startup; warnings are persisted to the audit CSV and defaults backfill any missing keys.

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
1. Fork the repository and create a feature branch. The `main` branch is protected and direct pushes are blocked, so branch-based workflows are mandatory.
2. Follow Clean Code principles—small, intention-revealing functions; avoid duplication; document intent when non-obvious.
3. Maintain Clean Architecture boundaries (use cases depend on ports, not concrete drivers).
4. Add or update unit tests under `tests/` ensuring >80% coverage on business logic.
5. Run `ctest` and the CAN simulator (if applicable) before submitting a pull request.
6. Update documentation and diagrams when behaviour or architecture changes.
7. All pull requests must pass the protected CI workflow (`Code Review Checks` and `Build and Test`); merges are blocked until every required check succeeds.

## Next Steps
- Integrate hardware-specific CAN, display, and touch drivers using ESP-IDF drivers or vendor libraries.
- Extend OTA workflow with signed manifests and delta updates.
- Build richer UI components (themes, trip history) leveraging the existing Clean Architecture foundation.
