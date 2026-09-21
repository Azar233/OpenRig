# Contributing

## Before coding

1. Read `docs/architecture.md`, `docs/engineering-standard.md`, and `docs/testing.md`.
2. Select one item from `todolist.md`; mark it `IN PROGRESS` and add your name/date before implementation.
3. For changes to the graph model, `AudioNode` ABI, parameter model, ownership, preset schema, or plugin parameter strategy, add an ADR first.

## Change requirements

- Keep commits focused and use an imperative subject (`Add linear graph validation`).
- Add or update tests with behavior changes.
- Run configure, build, CTest, and any task-specific checks.
- Update `todolist.md` with status and evidence (test name, benchmark path, or artifact).
- Never commit generated build output, licensed model files, commercial IRs, credentials, or machine-specific absolute paths.

## Review checklist

- Dependency direction is preserved.
- `process()` remains `noexcept`, bounded, allocation-free and lock-free.
- Public preset keys and node type strings remain stable.
- Buffer sizes are checked against `PrepareSpec::maxBlockSize`.
- Channel layouts, latency and tail behavior are explicit.
- Error handling is performed at a non-realtime boundary.
- Test coverage matches `docs/testing.md`.

## Definition of Done

A task is `DONE` only when its code, tests, documentation, and acceptance evidence are present. Code that compiles but lacks required tests stays `IN REVIEW` or `BLOCKED`.
