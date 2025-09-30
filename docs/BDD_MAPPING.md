# BDD Mapping Guide

This document maps high-level BDD scenarios to source areas and current implementation status. It serves as a living index so changes to firmware/UI logic keep the specs accurate.

## Tag Legend
- `@energy`, `@clock`, `@weather`, `@settings`: App domains
- `@core`: Always-green minimal regression subset
- `@wip`: Expected to fail or pending implementation
- `@integration`: Real hardware/network interactions (future)
- `@visual`: Display/animation/layout semantics (tolerances, transitions)
- `@calc`: Pure calculation logic (e.g., ice risk heuristic)

## Energy App
| Behaviour | Scenario Reference (feature) | File(s) | Status |
|-----------|------------------------------|---------|--------|
| Mode cycling & display semantics | energy_app.feature (multiple) | provisioning_server.c, energy_app.c, bridge.py | Implemented / passing |
| Peak detection (day/session) | energy_app.feature | energy_app.c / mock bridge | Passing |
| Colour state thresholds | energy_app.feature | energy_app.c | Passing |
| Daily reset & baseline pulses | energy_app.feature | energy_app.c | Passing |
| Auto-revert after inactivity | energy_app.feature | energy_app.c / UI mgr | Passing |

## Clock App
| Behaviour | Scenario | File(s) | Status |
|-----------|----------|---------|--------|
| Time ticking (seconds/minutes rollover) | clock_app.feature | clock_app.c (future), clock_steps.py | Passing (simulated) |
| Date popup on midnight boundary | clock_app.feature | clock_app.c (future) | Passing (sim) |
| 12/24 toggle via encoder | clock_app.feature | clock_app.c | Passing (sim) |
| Display type cycling (tap) | clock_app.feature | clock_app.c/UI | Passing |
| Nixie visual transitions | @wip scenarios | (planned display layer) | Pending |
| Power idle / pause | @wip | power mgmt module | Pending |
| Timezone resync | @wip | clock sync module | Pending |

## Weather App
| Behaviour | Scenario | File(s) | Status |
|-----------|----------|---------|--------|
| Current conditions (icon/temp/text/feels) | weather_app.feature @current | weather_app.c (future), weather_steps.py | Failing (placeholder) |
| Hourly ring (8 hours + wrap) | @hourly, @wrap | weather_app.c / UI radial | Failing (placeholder) |
| Multi-day toggle | @multiday | weather_app.c | Failing (placeholder) |
| Polling / interval logic | @polling @wip | controller | Pending |
| Stale indicator | @stale | weather_app.c | Failing (placeholder) |
| Icon mapping & fallback | @icons | asset mapping | Partial (fallback passes) |
| Partial data resilience | @resilience | weather_app.c | Failing (partial placeholders) |
| Network failure backoff | @network @wip | network module | Pending |
| Mode restore | @mode @wip | weather_app.c | Pending |
| Ice risk prediction | @ice | calc heuristic in steps | Passing (sim calc) |
| Radial positional semantics | @radial @wip | UI radial layout | Pending |

## Settings App
| Behaviour | Scenario | File(s) | Status |
|-----------|----------|---------|--------|
| Initial group header display | @list @core | settings_steps.py / settings_screen.c | Passing (BDD model) |
| Forward cycling across groups | @cycle @core | settings_steps.py | Passing |
| Ordered list integrity | @order @core | settings_steps.py | Passing |
| Group header no value field | @integrity @core | settings_steps.py | Passing |
| N-tap advance positioning | @advance @core | settings_steps.py | Passing |
| Wrap-around navigation | @wrap @wip | settings_steps.py | Pending |
| Backwards navigation (long press) | @back @wip | settings_steps.py | Pending |
| Editing numeric value | @edit @wip | (future settings_store) | Pending |
| Persistence of last item | @persist @wip | settings_store / NVS | Pending |
| Sensitive value masking | @value @wip | settings_screen.c | Pending |

## Future: Persistence Layer
Planned module: `settings_store.{c,h}` to abstract load/save via NVS (or other KV). BDD will evolve to assert persistence effects once foundation exists.

## Maintenance Workflow
1. Add scenario with @wip tag first.
2. Run targeted tag to verify syntax (e.g. `behave --tags=@settings`).
3. Implement code; update mapping table status from Pending → Passing.
4. Remove @wip tag only after green run.
5. Keep this doc updated in same commit as feature/status changes.

## Open Items Summary
- Weather: convert placeholder assertion failures to NotImplemented for cleaner pending signaling.
- Clock: implement timezone & power idle.
- Settings: introduce persistence & backward/edit flows.
- Visual tests: add tolerance helpers for radial/nixie layout before untagging @visual/@radial.

---
Generated initial version; refine as modules are added.
