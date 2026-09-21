# Test strategy and quality gates

Tests are part of implementation, not a cleanup phase. Every result must be reproducible from a recorded command, fixture and build configuration.

## Test layers

| Layer | Scope | Required evidence |
|---|---|---|
| Unit | DSP primitives, nodes, queues, serializers | deterministic pass/fail and numerical tolerance |
| Graph/component | validation, order, buffers, parameters, swaps, resources | positive and negative topology cases |
| Integration | iPlug2 audio adapter, device, NAM, IR, preset, plugin state | host/device/version matrix |
| Golden audio | sound-affecting regression | input WAV, reference WAV, algorithm/version metadata, tolerance |
| Realtime stress | allocations, exceptions, memory safety, swaps | block count, seed, sanitizer/guard result |
| Benchmark | mean/p95/p99 and realtime factor | machine/toolchain/config metadata |
| Soak/release | continuous end-to-end processing | duration, sample rate/block, dropout/crash/deadlock counters |

The dependency-free `openrig_core_tests` executable is a bootstrap harness. Adopt a pinned Catch2 version when test volume justifies it; do not delay early tests on that dependency.

## Mandatory node input suite

Every audio node must process, at minimum:

- silence;
- one-sample impulse at multiple offsets;
- positive and negative DC;
- sine at 100 Hz, 1 kHz and 10 kHz where below Nyquist;
- seeded noise;
- full-scale and modest over-range input;
- block lengths 1, a typical length, a non-power-of-two length, and `maxBlockSize`.

Common assertions: no crash, no exception, finite output, no unexpected DC, deterministic reset, correct bypass, parameter bounds, and no write beyond the declared frame/channel region.

## Node-specific assertions

- Gain: exact or tolerance-bounded multiplication and smoothing duration.
- Filters: magnitude/phase response at selected frequencies and stability.
- Delay: impulse timing, feedback decay, mix semantics and tail.
- Dynamics: detector timing, threshold/ratio curve and silence recovery.
- Modulation: rate/depth bounds and channel layout.
- Nonlinear/amp: finite output, alias/oversampling comparison, DC behavior and level bounds.
- Convolution: impulse equals IR within tolerance, latency report, IR swap behavior.
- NAM: supported sample-rate behavior, prewarm off audio, deterministic model swap and missing/corrupt file errors.

## Graph tests

Positive cases: empty passthrough, one node, ordered multi-node chain, bypass, variable blocks, mono/stereo declared conversions, latency/tail aggregation.

Negative cases: reserved/duplicate ID, unknown type, missing endpoint, duplicate edge, self-edge, branch/merge before supported, cycle, disconnected component, unsupported channel layout, over-max block.

Graph-swap stress must continuously process while a control producer publishes valid graphs and drains retired graphs. Acceptance: no allocation or lock on audio, no use-after-free, no leak, no corruption, deterministic final ownership.

## Golden audio policy

- Store small redistributable fixtures only; record their license/provenance.
- Reference metadata includes node type/version, sample rate, block sequence, parameter state, compiler and tolerance method.
- Prefer signal metrics (max absolute error, RMS error, correlation, spectral envelope) over byte equality when floating-point/compiler variation is expected.
- A deliberate sonic change updates references only with review notes explaining the audible/algorithmic change.

## Realtime safety

Each node must survive at least 100,000 process blocks under the realtime allocation guard. The core stress suite combines seeded parameter traffic and graph swaps. Target results:

- zero realtime heap allocation;
- zero exception;
- zero lock/wait;
- zero invalid memory access/race in available sanitizer builds;
- no non-finite samples unless the test explicitly injects non-finite input and defines recovery.

## Performance protocol

Standard scenarios:

| Sample rate | Block | Deadline |
|---:|---:|---:|
| 48 kHz | 64 | 1.333 ms |
| 48 kHz | 128 | 2.667 ms |
| 48 kHz | 256 | 5.333 ms |
| 96 kHz | 128 | 1.333 ms |

Report warmup, iterations, mean, p95, p99, maximum, realtime factor (`block duration / processing duration`), CPU, OS, power mode, compiler, flags and commit. The default chain must remain under 50% of its callback deadline on the reference machine; under 25% is the design target. A benchmark is not comparable if its environment metadata differs without annotation.

## Release candidate gate

Minimum standalone soak: 48 kHz, 128 samples, 30 minutes, with zero crash, deadlock, graph corruption or realtime allocation. Record driver/interface, measured dropouts, CPU and OS because device/OS scheduling is external to the DSP result.

Before a v0.1 release candidate:

- all P0 tasks for the milestone are `DONE`;
- Debug and Release build/tests pass from a clean tree;
- dependency/license register is current;
- preset compatibility and corrupt-input tests pass;
- VST3 validation/host matrix passes when plugin target exists;
- performance and soak reports are attached under `artifacts/` in CI or the designated release store (not committed when large).

## Test case record template

```text
ID:
Requirement/task:
Build/commit:
Environment:
Preconditions/fixtures:
Procedure:
Expected:
Actual:
Result: PASS | FAIL | BLOCKED
Evidence:
Owner/date:
```
