# Test Plan

## Objectives
- Validate correct decoding of CAN frames into `VehicleData` entities.
- Ensure alert thresholds trigger and reset under the right conditions.
- Confirm CSV logging adheres to schema and persists across sessions.
- Verify OTA workflow detects manifests and enforces version checks.
- Exercise UI navigation and manual access (simulated touch events).

## Test Strategy
- **Unit Tests** (`ctest`): Cover use cases (`CanDataProcessor`, `AlertEvaluator`, `DataLogger`, `OtaCoordinator`).
- **Integration Tests** (future): Hardware-in-the-loop with ESP32, CAN transceiver, and touch display.
- **Simulation Tests**: `tools/util/can_simulator.py` generates deterministic sequences for regression testing.

## Test Cases

| ID | Description | Steps | Expected Result |
|----|-------------|-------|-----------------|
| UC-CAN-001 | Decode speed frame | Feed frame `0x100` with payload `0x64` | `VehicleData.fSpeedKph = 100` |
| UC-CAN-002 | Decode RPM frame | Feed frame `0x101` with `0x20 0x0F` | `VehicleData.fEngineRpm = 3872` |
| UC-ALERT-001 | Trigger speed alert | Configure limit 120 km/h, feed 130 km/h | Alert `speed_high` active |
| UC-ALERT-002 | Reset speed alert | After alert, feed 100 km/h | Alert transitions to inactive |
| UC-LOG-001 | Write CSV header | First log invocation | Header row appended |
| UC-LOG-002 | Append log row | Subsequent log | CSV row with timestamp and values |
| UC-OTA-001 | Manifest available | Provide manifest with higher version | `OtaStatus.bUpdateAvailable = true` |
| UC-OTA-002 | Reject mismatched version | Call `downloadAndInstall` with unknown version | Returns `false` |
| UI-MAN-001 | Manual access | Simulate swipe to manual view | Manual content displayed |

## Test Data
- Sample CAN frames defined in `tests/unit/AlertEvaluatorTests.cpp` and `tools/util/can_simulator.py`.
- OTA manifest at `data/config/ota_manifest.txt`.

## Entry & Exit Criteria
- **Entry**: Code compiles, unit tests implemented for new features, logging enabled.
- **Exit**: 100% unit test pass, manual spot-check of simulator output, CSV validated.

## Reporting
- Use CTest output and CSV logs located in `build/Testing` and `data/logs` respectively.
- Capture simulator output for regression comparisons.
