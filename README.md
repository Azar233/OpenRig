# OpenRig

OpenRig is a modular, real-time guitar DSP platform. The first product target is a Windows x64 standalone application; VST3/CLAP and WebAssembly are later adapters over the same C++20 DSP core.

The repository starts with the architectural skeleton agreed in the project research conversation:

- `core/` owns audio contracts, graph compilation, parameter transport, realtime primitives, presets and resource boundaries.
- `dsp/` owns framework-independent signal-processing nodes.
- `framework/iplug/` will contain the iPlug2 adapter only; iPlug2 types must not leak into Core or DSP.
- `apps/` contains product entry points.
- `integrations/` will isolate NAM and WDF third-party APIs.
- `ui/` never holds or calls an `AudioNode*` directly.

## Current status

This is the M1 skeleton, not a released guitar processor. It includes a buildable Core/DSP layout, a v0.1 linear Graph compiler, preallocated graph buffers, parameter-event transport, realtime-scope instrumentation, example Gain/SoftClip nodes, and dependency-free smoke tests. Audio-device and iPlug2 integration remain intentionally pending.

Progress is authoritative in [`todolist.md`](todolist.md). Architecture and quality gates are documented under [`docs/`](docs/README.md).

## Build

Prerequisites: CMake 3.24+ and Visual Studio 2022 with the Desktop development with C++ workload. The supplied preset targets the first supported platform, Windows x64.

```powershell
cmake --preset dev
cmake --build --preset dev-debug
ctest --preset dev-debug
```

For another compiler or platform, use an explicit installed CMake generator:

```powershell
cmake -S . -B build/local -DOPENRIG_BUILD_TESTS=ON
cmake --build build/local --config Debug
ctest --test-dir build/local -C Debug --output-on-failure
```

## Non-negotiable realtime boundaries

1. `GraphDescription` is editable control-thread state; `CompiledGraph` is immutable realtime execution state.
2. Parameter changes do not rebuild the graph.
3. File I/O, parsing, allocation, NAM prewarm and IR preprocessing never occur in `process()`.
4. The audio thread does not allocate, lock, wait, log to disk, destroy heavy objects, or call UI code.
5. New third-party dependencies require a pinned revision, license record, ADR where architectural, and passing tests.

See [`docs/engineering-standard.md`](docs/engineering-standard.md) before changing public Core interfaces.
