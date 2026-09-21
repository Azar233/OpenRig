# Source conversation: accepted decisions

Source: [shared architecture research conversation](https://chatgpt.com/share/6ab08468-328c-83ee-a532-86e82784e7fe).

This document records decisions adopted by the repository. It is not a verbatim copy and does not treat product/library claims in the conversation as permanently current; licenses, APIs and platform support must be re-verified at dependency-integration time.

## Product intent

- A free/low-cost, modular guitar rig rather than a single fixed effect.
- Desktop first, then VST3/CLAP, then Web/WASM.
- v0.1 presents a linear pedalboard while the underlying model is graph-ready.
- MVP signal chain: input gain, gate, compressor, overdrive, amp, cabinet IR, chorus, delay, reverb, output gain, tuner and presets.
- Defer arbitrary routing, graph feedback, VST hosting, cloud, accounts, mobile, NAM training and a marketplace.

## Technical baseline

- C++20 and CMake.
- iPlug2 at the platform/plugin boundary, not inside Core/DSP.
- Self-owned DSP core and graph/parameter/resource systems.
- NAM Core behind `integrations/nam/`; WDF behind `integrations/wdf/`.
- Planar float32 internal audio.
- JSON preset schema with versioning and stable string identifiers.
- Windows x64 standalone is the first runnable product target.

## Architectural invariants

- Editable graph and realtime graph are separate objects.
- Graph swaps occur on block boundaries through staged publication; old graphs are reclaimed off the audio thread.
- Parameters flow through a parameter engine and smoothing, never through UI-owned DSP pointers.
- NAM/IR/preset resources are loaded, parsed, prepared and prewarmed on workers, then staged for swap.
- Graph scratch buffers are allocated during compilation.
- Core/DSP build and test without iPlug2.

## Milestones

- M1 Core host skeleton
- M2 Basic effects
- M3 Amp and cabinet
- M4 NAM
- M5 Production UI
- M6 VST3/CLAP
- M7 Web/WASM
