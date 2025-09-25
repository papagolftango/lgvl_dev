# Architecture

This firmware follows an "apps over an IoT OS" mental model:

- Managers: cross-cutting services (power, time, MQTT, LVGL, touch/display)
- Apps (Energy, Clock, Home, Settings, Weather): independent components using manager APIs
- UI: LVGL layers and screens owned by apps; input is routed via app manager

Key decisions:

- Power stability: LEDC backlight fades are serialized and wrapped with PM no-light-sleep locks.
- Input model: Touch switches apps globally; rotary is app-specific. Wake inputs are consumed.
- Time services: SNTP init (deprecated API currently) with day/hour callbacks.
- Energy domain: MQTT pulse counting with baselines; default center shows kWh Today; optional £ Today via hard-coded tariff.

See Testing & QA for how behavior is asserted.
