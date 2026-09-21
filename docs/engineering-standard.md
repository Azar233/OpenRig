# OpenRig Engineering Standard v0.1

Terms `MUST`, `MUST NOT`, `SHOULD` and `MAY` are normative.

## Language and naming

- Code MUST use C++20 without compiler extensions.
- Types use `PascalCase`; functions/variables use `camelCase`; constants use `kPascalCase`; namespaces are lowercase.
- Public code lives under `openrig` and narrower namespaces such as `openrig::graph` or `openrig::dsp`.
- Core samples are `float`; third-party `double` conversion stays in its adapter.
- `1.0F` and `-1.0F` represent full scale. Core MUST NOT silently normalize, limit, auto-gain or clip.

## Public contracts

- `NodeId` is `uint64_t`; value `0` is reserved. JSON stores it as a fixed-width hexadecimal string, never a JSON number.
- Public node type strings use stable namespaced values such as `openrig.drive.soft_clip`.
- Public parameter keys are stable once shipped. Runtime `ParameterIndex` values are not persistence identifiers.
- `AudioBlockView` is a non-owning view and MUST NOT allocate, resize or free.
- Nodes MUST accept variable block lengths up to `PrepareSpec::maxBlockSize`.
- `process()` and reset/parameter operations used by audio MUST be `noexcept`.
- Node polymorphism MAY dispatch once per block; per-sample virtual calls MUST NOT be introduced.

## Realtime rules

Within the audio callback or code it calls, the following are forbidden:

- `new`, `delete`, `malloc`, `free`, container growth, string construction;
- mutexes, condition variables, futures, sleeps or blocking atomics;
- filesystem, stream/file I/O, JSON parsing, NAM/IR loading, network I/O;
- disk/console logging and UI calls;
- graph validation/compilation, node creation or heavy destruction.

Audio code SHOULD have a known upper bound. Any failure in the realtime path must degrade safely (silence, bypass, rejected event, or fixed diagnostic queue) and MUST NOT throw.

## Ownership

- `std::unique_ptr<T>` is the default owner.
- Raw pointers and references are non-owning.
- `std::shared_ptr<const T>` MAY represent shared immutable resources, but the audio thread MUST NOT perform the final destruction.
- Graph/resource replacement uses staged and retired queues; construction and destruction happen outside audio.

## Graph

- UI/control owns `GraphDescription`; audio owns only a ready `CompiledGraph`.
- Structural edits compile a new graph. Knob/switch changes do not.
- Compiler validation MUST reject duplicate/reserved IDs, missing endpoints, unknown node types, cycles, disconnected output, unsupported routing and channel layouts.
- v0.1 UI/compiler expose a linear connected graph; internal APIs remain graph-shaped.
- All node storage, scratch audio and schedules MUST be prepared before publication.

## Parameters

- UI, preset, host and later MIDI sources all produce `ParameterEvent` values.
- UI code MUST NOT retain `AudioNode*`.
- Continuous parameters SHOULD smooth. Starting guidance: gain/drive/mix 5–20 ms; frequency 10–50 ms.
- Boolean/choice/mode parameters update at block boundaries without interpolation.
- Queue overflow MUST be observable and handled; silently blocking the producer is forbidden.

## Resources and presets

- Nodes accept prepared resources, not filesystem paths.
- NAM prewarm, WAV decoding, IR resampling and convolution-plan creation happen on workers.
- Presets MUST contain `schemaVersion` and `appVersion`.
- Unknown node types become a bypassing `MissingNode` that preserves the original JSON for round-trip compatibility.
- External assets use `ResourceRef` rather than hard-coded machine paths. Resolution order is preset-relative, user resource library, then recorded absolute path.

## Error and logging policy

- Control/worker boundaries MAY use exceptions internally but MUST catch third-party exceptions before publishing state.
- Prefer structured result/error codes for expected failures.
- Audio errors enter a fixed-capacity diagnostic queue. A non-realtime consumer formats and persists them.
- Tests and debug builds SHOULD use `RealtimeScope` and an allocation hook to fail realtime allocations; the hook itself is tracked under T037.

## Third-party changes

- Pin versions/commits and update `THIRD_PARTY.md`.
- Verify license and transitive obligations at the selected revision.
- Architectural dependencies require an ADR.
- GPL reference code MUST NOT be copied into this repository absent an explicit licensing decision.

## Formatting and includes

- Headers use `#pragma once`.
- Include what is used and prefer standard headers after project headers.
- Avoid macros except platform boundaries and compile-time configuration.
- Keep generated/framework headers out of Core public headers.
