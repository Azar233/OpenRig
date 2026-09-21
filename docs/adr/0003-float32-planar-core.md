# 0003 Float32 planar audio core

Status: Accepted  
Date: 2026-09-21

## Context

Realtime guitar DSP, SIMD and NAM integrations favor predictable buffer layout and bandwidth. Some external APIs may use another representation.

## Decision

Core uses non-interleaved planar 32-bit float PCM. Adapters perform any interleaving or precision conversion.

## Consequences

Nodes share one efficient contract. Conversion cost stays visible at boundaries, and Core is not forced to adopt third-party precision choices.
