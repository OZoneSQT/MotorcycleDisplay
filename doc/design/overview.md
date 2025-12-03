# Project Layout

```text
project-root/
├── .gitignore
├── .env                    # environment variables
├── bus.env                 # CAN bus addresses
├── CMakeLists.txt          # build system config
├── README.md               # high-level project overview
├── LICENSE                 # license file
│
├── doc/                    # documentation bundle
│   ├── design/             # design notes and diagrams
│   ├── complience/         # compliance notes
│   ├── api/                # generated API docs (Doxygen)
│   └── user/               # user manuals, guides
│
├── tests/                  # unit and integration tests
│   ├── unit/               # fine-grained tests per module
│   ├── integration/        # system-level tests
│   └── mocks/              # mock objects for HAL/driver testing
│
├── src/                    # main source code
│   ├── controller/         # control logic, state machines
│   ├── driver/             # hardware drivers and stubs
│   ├── hal/                # hardware abstraction layer
│   ├── logic/              # core business/application logic
│   ├── networking/         # sockets, protocols, comms
│   ├── ota/                # OTA update mechanisms
│   ├── simulation/         # digital twin orchestration layer
│   └── ui/                 # dashboard rendering
│
├── include/                # public headers mirroring src/
│   ├── controller/
│   ├── driver/
│   ├── hal/
│   ├── logic/
│   ├── networking/
│   ├── ota/
│   ├── simulation/
│   └── ui/
│
├── data/                   # runtime assets
│   ├── config/             # configuration files and OTA manifests
│   └── manual/             # rendered manual content
│
├── tools/                  # scripts and utilities
│   ├── build/              # build helpers
│   ├── deploy/             # deployment scripts
│   ├── ci/                 # CI/CD helpers
│   └── util/               # developer utilities (simulators, etc.)
│
└── cmake/                  # custom CMake modules
```

## Simulation and Digital Twin Assets

- `tools/util/can_simulator.py` -- host-side generator for deterministic CAN sequences or CSV playback to exercise the pipeline without a bike.
- `include/driver/DigitalDisplayDriver.hpp` / `src/driver/DigitalDisplayDriver.cpp` -- in-memory display target that captures rendered gauges, alerts, and manual content for assertions.
- `include/simulation/` / `src/simulation/` -- digital twin harness that wires CAN processing, alert evaluation, logging, manual loading, and the display stub together.
- `tests/integration/DashboardDigitalTwinTests.cpp` -- integration coverage validating that simulator frames drive alerts, logs, and user manual rendering end-to-end via the twin.

### Recommended Simulation Workflow

1. Configure the host build once with `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug`.
2. Rebuild the test executable (`cmake --build build --target motorcycle_tests`) after local changes.
3. Execute `ctest --test-dir build --output-on-failure` to run both unit tests and the digital twin integration suite.
4. When iterating on UI or alert logic, drive additional scenarios through `tools/util/can_simulator.py` or custom frame injections via `simulation::DashboardDigitalTwin`.