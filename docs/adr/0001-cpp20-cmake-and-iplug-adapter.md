# 0001 C++20/CMake core with iPlug2 as an adapter

Status: Accepted  
Date: 2026-09-21

## Context

OpenRig needs standalone, plugin and later web entry points without duplicating DSP. Framework licensing and long-term portability also matter.

## Decision

Use C++20 and CMake. Keep iPlug2/iPlug2OOS in `framework/iplug`, application targets and UI. Core/DSP public headers cannot include iPlug2.

## Consequences

Core tests run without a plugin framework, and new hosts can reuse the same DSP. An adapter layer must explicitly translate buffers, parameters, state, latency and UI messages.
