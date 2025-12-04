Implementation/
├── CMakeLists.txt          # entrypoint for desktop builds
├── bus.env                 # CAN bus connection defaults (sample)
├── data/                   # runtime assets (manuals, CSV logs, manifests)
│   ├── config/
│   └── manual/
├── design/                 # quick-reference developer notes
├── include/                # public headers mirroring src/
│   ├── controller/
│   ├── driver/
│   ├── hal/
│   ├── logic/
│   ├── networking/
│   ├── ota/
│   ├── simulation/
│   └── ui/
├── src/                    # Clean Architecture implementation
│   ├── controller/
│   ├── driver/
│   ├── hal/
│   ├── logic/
│   ├── networking/
│   ├── ota/
│   ├── simulation/
│   └── ui/
├── tests/                  # unit/integration coverage
│   ├── integration/
│   ├── mocks/
│   └── unit/
├── tools/                  # scripts/utilities
│   ├── build/
│   ├── ci/
│   ├── deploy/
│   └── util/
├── typings/                # Python typing shims
├── pyrightconfig.json      # strict Python analysis configuration
├── requirements.txt        # Python dependencies (host tooling)
├── Run_Setup.cmd           # helper to launch Setup.ps1 under pwsh
└── Setup.ps1               # repo provisioning script (installs Python, CMake, Ninja; imports compiler toolchains)