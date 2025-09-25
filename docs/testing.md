# Testing & QA

We use a layered approach:

1. Unit tests: domain logic and adapters (Unity/CMock)
2. Contract tests: ports like MQTT, persistence (fakes in test mode)
3. BDD: user-visible behavior across managers and apps (Gherkin)

The BDD .feature files live in `tests/bdd/features`. A small generator copies them into `docs/bdd` so they appear here as living documentation.

Future work:
- CONFIG_TEST_MODE bridge for step definitions
- Presenter/ViewModel pattern for each app to make UI state queryable
