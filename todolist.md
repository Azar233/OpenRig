# OpenRig development ledger

Last updated: 2026-09-21  
Current milestone: **M1 — realtime host skeleton**  
Progress rule: only `DONE` counts as complete. A task needs code, required tests, documentation and acceptance evidence.

Statuses: `TODO` · `IN PROGRESS` · `BLOCKED` · `IN REVIEW` · `DONE`

## M1 — Realtime host skeleton

| ID | P | Status | Task | Acceptance criteria | Depends on | Notes / evidence |
|---|---:|---|---|---|---|---|
| T001 | P0 | DONE | Repository + CMake skeleton | Debug/Release configuration; Core/DSP/sandbox/tests targets; documented build | — | VS2022 x64 Debug + Release and warnings-as-errors build passed; CTest 1/1 passed in both configs; sandbox output `0.7` |
| T002 | P0 | TODO | Pin and integrate iPlug2OOS | Exact commit/license recorded; Windows standalone target generated | T001 | Do not leak iPlug headers into Core/DSP |
| T003 | P0 | TODO | Audio passthrough | Select ASIO device; mono guitar input reaches output stably | T002 | Record device/sample-rate/block evidence |
| T004 | P0 | DONE | Independent `openrig_core` library | Builds/tests without iPlug2 or UI | T001 | `openrig_core.lib` built in Debug/Release with no iPlug2/UI dependency |
| T005 | P0 | IN REVIEW | `AudioNode` contract + Gain node | Gain runs for variable blocks; bypass/reset/descriptor tests pass | T004 | Initial implementation and unity test added |
| T006 | P0 | IN REVIEW | Buffer/process contracts | Non-owning planar views; no process-time allocation; bounds documented | T004 | `AudioBlockView`, `PrepareSpec`, `ProcessContext` added |
| T007 | P0 | IN REVIEW | Parameter event engine | UI/control producer can address node parameter; overflow observable; audio drain bounded | T005 | SPSC transport skeleton added; graph-to-node routing remains |
| T008 | P0 | IN REVIEW | Parameter smoothing | Gain/drive changes ramp over declared duration; zipper regression test | T007 | `LinearSmoother` implemented; duration regression test remains |
| T009 | P0 | IN REVIEW | `GraphDescription` | Stable nodes/connections describe persisted linear graph | T004 | Types added; serializer belongs to T021 |
| T010 | P0 | IN REVIEW | `GraphCompiler` | Valid chain compiles; duplicate/missing/cycle/disconnected/layout cases tested | T009, T013 | Initial validation implemented; full negative/channel test matrix remains |
| T011 | P0 | IN REVIEW | `CompiledGraph` execution | Preallocated buffers; variable block execution; latency/tail aggregation | T010 | Linear ping-pong implementation; tail/channel tests remain |
| T012 | P0 | TODO | Staged graph swap | SPSC publish + retire; no audio allocation/delete/lock; stress test | T011 | Follow ADR 0004 |
| T013 | P0 | IN REVIEW | Node registry/factory | Stable type creates a node; duplicate/unknown types handled | T004 | Initial registry and tests added |
| T021a | P0 | TODO | Minimal M1 preset round-trip | Save/load simple Gain/SoftClip graph with schema v1 | T009, T013 | Narrow M1 slice of T021 |
| T037a | P0 | TODO | Realtime allocation guard | Test fails if `process()` allocates; 100k block stress passes | T005, T011 | `RealtimeScope` API exists; allocation hook pending |
| T041 | P0 | TODO | M1 Windows CI | Clean configure/build/test for Debug + Release; artifacts retained | T001 | Split from full CI task for early feedback |

### M1 exit gate

- Standalone audio passthrough works at 48 kHz/128.
- Gain → SoftClip → Gain graph executes with live smoothed parameters.
- Add/delete/reorder compiles and swaps without audio-thread lock/allocation/destruction.
- Minimal preset round-trip succeeds.
- Unit/negative/realtime stress tests and a 30-minute smoke soak pass.

## M2 — Basic effects and usable pedalboard

| ID | P | Status | Task | Acceptance criteria | Depends on |
|---|---:|---|---|---|---|
| T014 | P1 | TODO | Biquad filter | LPF/HPF response and stability tests | M1 |
| T015 | P1 | TODO | Noise Gate | Threshold/attack/release tests; finite output | T014 |
| T016 | P1 | TODO | Compressor | Attack/release/threshold/ratio behavior tested | T014 |
| T017 | P1 | TODO | SoftClip Overdrive product node | Drive/Tone/Level and golden audio | T014, T008 |
| T018 | P1 | TODO | Delay | Time/feedback/mix; impulse/tail tests | M1 |
| T019 | P1 | TODO | Chorus | Rate/depth/mix; mono-to-stereo declaration | T018 |
| T020 | P1 | TODO | Reverb | Basic room/hall; stability/tail/performance tests | M1 |
| T031 | P0 | TODO | Basic pedalboard UI | Add/delete/bypass/reorder without direct node pointers | T002, T012 |
| T032 | P0 | TODO | Descriptor-driven parameter panel | Knob/switch/choice from descriptors; queue-only updates | T007, T031 |
| T033 | P1 | TODO | Input/output meters | Bounded audio-to-UI telemetry; no direct drawing from audio | T003 |
| T034 | P1 | TODO | Preset browser | Save/load/error/missing-resource UX | T021 |

## M3 — Amp and cabinet

| ID | P | Status | Task | Acceptance criteria | Depends on |
|---|---:|---|---|---|---|
| T021 | P0 | TODO | Full preset schema v1 | Versioned graph/resources; unknown-node round trip; migrations tested | T021a, M2 |
| T022 | P1 | TODO | IR loader | Supported WAV variants validated; corrupt/large input bounded | T027 |
| T023 | P1 | TODO | Partitioned convolver | Impulse/golden/latency/performance tests | T022 |
| T024 | P1 | TODO | Cabinet node | IR + HPF/LPF; staged resource swap | T014, T023 |
| T027 | P0 | TODO | Async ResourceManager | I/O/parse/prepare/prewarm off audio; errors observable | T012 |
| T028 | P1 | TODO | Procedural amp | Clean/Crunch with oversampling/alias benchmarks | T014, T017 |
| T029 | P1 | TODO | chowdsp_wdf integration | Pinned/license recorded; RC/diode demo tested | T001 |
| T030 | P2 | TODO | WDF overdrive | First circuit-level effect with golden/benchmark | T029 |

## M4 — NAM

| ID | P | Status | Task | Acceptance criteria | Depends on |
|---|---:|---|---|---|---|
| T025 | P0 | TODO | NAM Core integration | Pinned/license/transitives recorded; `.nam` loads off audio | T027 |
| T026 | P0 | TODO | `NamNode` | Ordinary graph node; sample-rate/prewarm/swap/latency tests | T025 |

## M5/M6 — Production UI and plugin

| ID | P | Status | Task | Acceptance criteria | Depends on |
|---|---:|---|---|---|---|
| T035 | P0 | TODO | VST3 build | Validated and loads in agreed DAW matrix | M2, T002 |
| T036 | P0 | TODO | Plugin state restore | DAW reopen reproduces graph/params/resources | T021, T035 |
| T037 | P0 | TODO | Full realtime safety audit | Zero callback allocation/lock; sanitizer/stress report | M4 |
| T038 | P0 | TODO | Benchmark suite | Node/default chain scenarios with mean/p95/p99/RTF | M2 |
| T039 | P1 | TODO | Full CI matrix | Windows build/test/package; later macOS as target exists | T041, T035 |

## M7 — Web/WASM

| ID | P | Status | Task | Acceptance criteria | Depends on |
|---|---:|---|---|---|---|
| T040 | P2 | TODO | WAM/Web prototype | Same DSP Core runs via WASM + AudioWorklet; latency documented | M6 |

## Backlog rules

- Do not start M2 effect volume before the M1 exit gate.
- Do not add NAM, polished production UI or a procedural amp to unblock M1.
- New work receives an ID, priority, dependency and measurable acceptance criteria before coding.
- When blocked, record the exact blocker and next decision/action in Notes; `BLOCKED` is not a substitute for an unclear task.
