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
