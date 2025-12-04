# Project Layout

```text
project-root/
├── .github/                  # workflows, Copilot guidance
├── Commercial/               # customer-facing collateral
├── Documentation/            # formal documentation bundle
│   ├── api/
│   ├── complience/
│   ├── design/
│   └── user/
├── Implementation/           # build-ready source tree
│   ├── CMakeLists.txt        # entrypoint for host builds
│   ├── data/                 # runtime assets (manuals, CSV logs, manifests)
│   ├── design/               # lightweight developer notes
│   ├── include/              # public headers mirroring source layout
│   ├── src/                  # Clean Architecture implementation
│   │   ├── controller/
│   │   ├── driver/
│   │   ├── hal/
│   │   ├── logic/
│   │   ├── networking/
│   │   ├── ota/
│   │   ├── simulation/
│   │   └── ui/
│   ├── tests/                # unit, integration, and harness coverage
│   │   ├── integration/
│   │   ├── mocks/
│   │   └── unit/
│   ├── tools/                # build, CI, deployment helpers, simulators
│   │   ├── build/
│   │   ├── ci/
│   │   ├── deploy/
│   │   └── util/
│   └── typings/              # custom Python stubs for tooling
├── Maintenance/              # service procedures and support notes
└── README.md                 # high-level project overview
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