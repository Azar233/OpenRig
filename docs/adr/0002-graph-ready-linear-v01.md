# 0002 Graph-ready model with a linear v0.1 product

Status: Accepted  
Date: 2026-09-21

## Context

A vector effect chain is simple, but future split/mix routing would require replacing its persistence and editing model.

## Decision

Represent user state as nodes and connections from day one. v0.1 validation/UI accepts exactly one connected linear chain. Graph feedback, branching and merging are deferred.

## Consequences

Preset identity and graph APIs survive later DAG work. The initial compiler carries some graph terminology while intentionally rejecting unsupported topologies.
