# Architecture

## Layer model

```text
apps (standalone/plugin/web)
        |
framework adapters + ui
        |
core (engine/graph/parameters/resources/presets/realtime)
        |
dsp (framework-independent nodes)
        |
integrations (NAM/WDF) and pinned foundational libraries
```

Allowed dependency direction is downward. `core/` and `dsp/` must compile without iPlug2, GUI, host SDK or filesystem-specific application code. Integrations adapt third-party APIs to Core contracts; third-party types do not cross their public boundary.

## Repository map

```text
apps/
  standalone/          temporary sandbox; later iPlug2 host
  plugin/              future VST3/CLAP entry point
framework/
  iplug/               future audio/plugin adapter
core/
  include/openrig/core/
    audio/              blocks, prepare/process contracts
    graph/              description, validation, compilation, execution
    parameter/          descriptors and event transport
    preset/             future schema mapping/migration
    realtime/           bounded lock-free primitives and guards
    resource/           async load boundary
dsp/
  include/openrig/dsp/  reusable DSP nodes and primitives
integrations/
  nam/                  future NAM adapter
  wdf/                  future chowdsp_wdf adapter
ui/                     future presentation layer
tests/                  deterministic tests
benchmarks/             realtime budgets and regression data
docs/adr/               architecture decisions
third_party/            pinned dependencies if vendored
```

Empty future directories should be added when their first owned artifact is introduced, not merely to decorate the tree.

## Runtime worlds

| World | Owns | May do | Must not do |
|---|---|---|---|
| Audio | active `CompiledGraph`, preallocated buffers, realtime parameter consumption | bounded DSP and atomic/lock-free exchange | allocate, lock, wait, file/network I/O, parse, log, draw UI, destroy heavy objects |
| Control/UI | `GraphDescription`, user edits, command state | allocate, validate, request compile, enqueue parameters | mutate active graph/nodes directly |
| Worker | NAM/IR decode, FFT plan, prepare/prewarm | file I/O, parsing, expensive construction | enter realtime processing |
| Background/IO | preset storage, scanning, non-RT logs | filesystem and indexing | own audio callback state |

## Graph lifecycle

```text
GraphDescription (control)
        |
validate: IDs/types/endpoints/layout/cycles
        |
topological order + node creation + prepare
        |
buffer allocation + processing schedule
        v
CompiledGraph B
        |
SPSC publish mailbox
        v (block boundary)
active A -> active B -> retire A
                         |
                    SPSC retire queue
                         |
                 control-thread destruction
```

v0.1 accepts a single connected linear chain. The types deliberately retain nodes and connections so parallel DAG routing can be added later without changing preset identity. Graph-level feedback is invalid; a delay/reverb may maintain bounded feedback internally.

The checked-in `GraphCompiler` is an initial linear implementation. Its next steps are channel-capability validation, persisted parameter application, structured error codes, graph-swap ownership, and DAG buffer lifetime analysis only when parallel routing is authorized.

## Parameter lifecycle

```text
UI / preset / host macro / future MIDI
        |
ParameterEvent { NodeId, ParameterIndex, value }
        |
bounded SPSC queue
        |
audio block boundary
        |
node setParameter -> smoother/discrete latch -> process
```

Runtime indices may change between builds. Persistence uses stable node type strings and parameter keys. Continuous controls define smoothing; booleans, choices and integer modes latch at block boundaries.

## Resource lifecycle

Nodes never open files. `ResourceManager` receives a request, a worker loads and validates the resource, prepares/prewarms an immutable object, and publishes it for a block-boundary swap. Final destruction must be forced onto a non-realtime thread, including the last release of shared immutable resources.

## Buffer/channel policy

- Planar/non-interleaved float32 PCM.
- Actual callback frames may vary and must be `<= maxBlockSize`.
- v0.1 guitar input is mono; internal nodes declare mono-to-mono, mono-to-stereo or stereo-to-stereo; product output is stereo.
- Compiler owns routing and memory. Nodes receive non-owning views and never resize buffers.
- Linear chains use two preallocated ping-pong scratch buffers; in-place support is an explicit capability, not an assumption.

## Platform adapters

The future iPlug2 call path is:

```text
iPlug2 ProcessBlock
  -> IPlugAudioAdapter
  -> AudioEngine::process
  -> active CompiledGraph
```

Host parameter exposure is intentionally limited in v0.1 to input/output gain and a fixed macro bank. Dynamic internal nodes do not force unstable dynamic VST parameter identities into Core.
