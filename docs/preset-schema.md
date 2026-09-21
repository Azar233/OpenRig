# Preset schema v1 contract

The serializer is planned in T021. This document freezes the compatibility shape before implementation.

```json
{
  "schemaVersion": 1,
  "appVersion": "0.1.0",
  "name": "Modern Lead",
  "inputGainDb": 0.0,
  "outputGainDb": -3.0,
  "nodes": [
    {
      "id": "000000000000001A",
      "type": "openrig.drive.soft_clip",
      "bypassed": false,
      "parameters": {
        "drive": 0.42,
        "level": 0.72
      }
    }
  ],
  "connections": [],
  "resources": []
}
```

## Rules

- `schemaVersion` is an integer used by migrations; `appVersion` is informational/diagnostic.
- `NodeId` is a 16-character hexadecimal string to preserve all 64 bits across JavaScript/Web.
- Node `type` and parameter keys are stable public identifiers.
- Connections are persisted even though v0.1 exposes only a linear chain.
- Unknown top-level and node fields should be retained when feasible.
- Unknown node types load as bypassing `MissingNode` objects and preserve their original JSON.
- Missing external resources do not abort the whole preset; the owning node enters a visible missing-resource state.

## Resource reference

```json
{
  "kind": "nam",
  "path": "models/5150.nam",
  "sha256": "optional-lowercase-hex"
}
```

Resolution order is preset-relative path, OpenRig resource library, then a recorded absolute fallback if present. Saved presets should prefer portable relative/library references. No model or IR bytes are embedded by default.

## Migration policy

- Parsers validate structure and limits before constructing a graph.
- Every schema change adds fixture-based migration and round-trip tests.
- Loading an older supported version produces the current in-memory form; saving writes the current version unless an explicit compatibility export is requested.
- A newer unknown schema version must produce a clear non-realtime error and must not partially activate audio state.
