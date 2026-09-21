# 0004 Staged graph and resource swaps

Status: Accepted  
Date: 2026-09-21

## Context

Graph compilation, NAM loading/prewarm, IR preprocessing and object destruction are not realtime safe. A callback mutex can cause dropouts or priority inversion.

## Decision

Prepare replacement state off the audio thread, publish through a bounded SPSC mailbox, swap only at a block boundary, and return the replaced object through a retire queue for non-realtime destruction.

## Consequences

Audio processing does no construction, locking or heavy destruction. Ownership and queue-full behavior require explicit tests, and reclamation must be serviced regularly by control code.
