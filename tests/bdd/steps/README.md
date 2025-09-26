# Step Definitions

These features assume a simple test bridge (enabled via CONFIG_TEST_MODE) to drive and inspect the device during BDD runs.

Suggested minimal endpoints or harness APIs:

- App manager
  - GET/POST active app: get/set by name or ID
- Input injection
  - Touch: tap()
  - Encoder: rotate("LEFT"|"RIGHT")
- Power/time
  - Set inactivity timeout; force IDLE/ACTIVE; advance time
  - Read back state and backlight level
- Haptics
  - Read last emitted flag/timestamp
- Energy ViewModel (for richer assertions later)
  - GET /test/energy/viewmodel → { title, value_text, mode, color }

You can implement the bridge over HTTP or UART. For headless, link step defs directly to a C harness compiled for host.

## How to run (mock only)

Run all features locally against the in-memory mock (no device required):

- PowerShell (from repo root):
  - tools/run-bdd.ps1

Or, if you already have behave installed and prefer direct invocation:

- PowerShell (from tests/bdd):
  - behave

Optional flags:

- Filter by tags (e.g., wakeup-only):

  - tools/run-bdd.ps1 -Tags @wakeup

- Syntax check only (no execution):

  - tools/run-bdd.ps1 -DryRun

## Run against a device later

Once CONFIG_TEST_MODE endpoints are available, switch the bridge:

- HTTP bridge:

  - tools/run-bdd.ps1 -BridgeMode http -BridgeUrl http://DEVICE_IP:PORT

- Serial bridge:

  - tools/run-bdd.ps1 -BridgeMode serial -SerialPort COM3 -SerialBaud 115200

