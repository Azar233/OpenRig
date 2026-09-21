# Development workflow

## Toolchain

- CMake 3.24 or later
- C++20 compiler: current MSVC is the first supported toolchain; Clang/GCC are portability targets
- Ninja for checked-in presets, or another explicit CMake generator
- Git

iPlug2, NAM, WDF and JSON dependencies are not fetched by the current skeleton. Add each only in its tracked task after pinning and license review.

## Task flow

1. Choose an unblocked task in `todolist.md`.
2. Change its status to `IN PROGRESS`, record owner/date, and keep acceptance criteria unchanged unless the scope decision is documented.
3. Add or amend an ADR before changing an architectural boundary.
4. Implement the smallest vertical slice that meets acceptance criteria.
5. Run required tests and capture evidence in the task's Notes/Evidence column.
6. Mark `IN REVIEW`; after review and all checks, mark `DONE`.

Allowed statuses are `TODO`, `IN PROGRESS`, `BLOCKED`, `IN REVIEW`, and `DONE`. Never report percentage complete from intuition; milestone progress is `DONE tasks / milestone tasks`, with P0 blockers called out separately.

## Branch and commit convention

Suggested branches: `feat/T007-parameter-engine`, `fix/T010-cycle-detection`, `docs/ADR-0005-resource-lifetime`.

Commit subjects are imperative and scoped. Do not mix formatting, dependency upgrades and behavior changes in one commit.

## Adding a node

1. Select a stable namespaced node type and parameter keys.
2. Define descriptors, channel capabilities, latency/tail behavior and default bypass behavior.
3. Allocate all state in `prepare()`; handle variable callback length.
4. Implement bounded `noexcept` process code and explicit reset.
5. Register through `NodeRegistry`; do not add a type-switch chain.
6. Add the tests specified in `testing.md`, benchmark if nontrivial, then document it in the task ledger.

## Changing the graph or preset schema

These changes require an ADR, migration/compatibility plan, negative tests, and an update to `docs/preset-schema.md`. Once released, node types and parameter keys are compatibility contracts.

## Local verification

```powershell
cmake --preset dev
cmake --build --preset dev-debug
ctest --preset dev-debug
```

For release-sensitive DSP, also run Release tests and benchmark scenarios in `docs/testing.md`.
