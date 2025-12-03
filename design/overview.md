project-root/
├── .gitignore
├── .env					# enviorment variabels
├── CMakeLists.txt          # build system config
├── README.md               # high-level project overview
├── LICENSE                 # license file
│
├── docs/                   # documentation
│   ├── design/             # design notes
│   │   ├── diagrams/       # architecture diagrams
│   │   ├── complience/     # complience notes
│   │   ├── enclosure/      # enclosure notes
│   │   ├── hardware/       # hardware notes
│   │   └── user/           # user manuals, guides
│   │
│   ├── api/                # generated API docs (Doxygen)
│   └── user/               # user manuals, guides
│
├── tests/                  # unit/integration tests
│   ├── unit/               # fine-grained tests per module
│   ├── integration/        # system-level tests
│   └── mocks/              # mock objects for HAL/driver testing
│
├── src/                    # main source code
│   ├── ui/                 # CLI, GUI, or embedded UI
│   ├── ota/                # OTA update mechanisms
│   ├── networking/         # sockets, protocols, comms
│   ├── logic/              # core business/application logic
│   ├── controller/         # control logic, state machines
│   ├── hal/                # hardware abstraction layer
│   └── driver/             # hardware drivers
│
├── include/                # public headers
│   ├── ui/
│   ├── ota/ 
│   ├── networking/
│   ├── logic/
│   ├── controller/
│   ├── hal/
│   └── driver/
│
├── data/                   # data
│   ├── config/             # configuration
│   └── manual/             # manual data
│
├── tools/                  # scripts/utilities
│   ├── build/              # build helpers
│   ├── util/               # utilities
│   ├── deploy/             # deployment scripts
│   └── ci/                 # CI/CD helpers
│
└── cmake/                  # custom CMake modules