# Motorcycle Dashboard User Manual

## Getting Started
- Power on the motorcycle and allow the ESP32 display to boot. The dashboard automatically connects to the CAN bus and begins streaming data.
- Touch anywhere on the dashboard home screen to reveal the quick actions bar.

## Navigating the Dashboard
- **Home View**: Displays live speed, RPM, throttle, ABS status, engine temperature, and battery voltage.
- **Manual View**: Swipe right to open the built-in manual. Swipe left to return to the live dashboard.
- **Alerts View**: Tap the alert icon in the upper-right corner to review active alerts and their timestamps.

## Configuring Alerts
1. Open the alerts view.
2. Tap **Configure Alerts**.
3. Adjust the threshold sliders for speed, RPM, and engine temperature.
4. Save your changes. The system immediately applies the new configuration.

## Viewing Logged Data
- The system stores CSV logs at `/logs/vehicle_log.csv` on the ESP32 storage.
- Connect the device over USB or Wi-Fi and download the file for analysis.
- Columns include timestamp, speed, RPM, throttle position, ABS state, engine temperature, and battery voltage.

## Bench Testing with the Digital Twin
- A desktop digital twin mirrors the dashboard pipeline so you can validate layouts and alerts without hardware.
- Build the host project with CMake and run the integration tests:
	- `cmake --build build --target motorcycle_tests`
	- `ctest --test-dir build --output-on-failure`
- Inspect the test output to verify the `DashboardDigitalTwin` renders expected values and alerts via the in-memory display driver.
- Optionally, run the Python CAN simulator (`tools/util/can_simulator.py`) to stream scripted telemetry into the twin for richer scenarios.

## Manual Access on Dashboard
- Swipe right from the home view to show the manual.
- Scroll vertically to read details about controls and safety notes.
- Tap **Close** to return to the dashboard.

## OTA Updates
- When an update is available, a badge appears on the settings icon.
- Tap **Settings → Updates → Install** to apply the latest firmware.
- Ensure the bike remains powered during the update process.

## Support
- Refer to the project README for setup and troubleshooting.
- Logs and alert history help diagnose issues—include them when reporting problems.
