# Tests

This repository uses a layered test strategy:

- Unit tests (Unity/C): Fast tests for domain logic, presenters, and manager contracts using fakes.
- Contract/Conformance tests (Unity/C): Verify adapters conform to port interfaces.
- BDD tests (Gherkin): End-to-end behaviors expressed as Given/When/Then, asserting ViewModels and system state.

Folder layout:

- `tests/unit/`          — C unit tests (Unity) for domain, presenter, managers
- `tests/contracts/`     — Port/adapters conformance suites
- `tests/bdd/features/`  — Gherkin feature files grouped by domain
- `tests/bdd/steps/`     — Step definitions (Python or C harness), talking to a CONFIG_TEST_MODE bridge

Getting started:

1) Start with BDD for overall app behavior in `tests/bdd/features/app/` (navigation, rotary, power, haptics).
2) Add minimal test hooks (CONFIG_TEST_MODE) to set state and retrieve ViewModels.
3) Backfill unit tests for domain/presenter.

This scaffold focuses on BDD first. We’ll add unit/contract trees as we implement presenters and ports.
