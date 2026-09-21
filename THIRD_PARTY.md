# Third-party dependency register

No external source dependency is vendored or fetched by the current skeleton.

Before adding a dependency, record the exact version/commit, upstream URL, license, purpose, owner, update method, and any redistribution obligations. Release builds must never track an unpinned `main`/`latest` branch.

| Dependency | Planned purpose | License to verify | Status |
|---|---|---|---|
| iPlug2 / iPlug2OOS | Standalone, VST3, CLAP and later WAM adapter | zlib-like; verify selected revision | Planned (T002) |
| NeuralAmpModelerCore | Neural amp inference | MIT; verify selected revision and transitive deps | Planned (T025) |
| Eigen | NAM linear algebra | MPL2 and bundled notices; verify configuration | Planned with NAM |
| chowdsp_wdf | Circuit/WDF models | BSD-3-Clause; verify selected revision | Planned (T029) |
| nlohmann/json | Preset serialization | MIT; verify selected revision | Planned (T021) |
| Catch2 | Unit/property tests | BSL-1.0; verify selected revision | Candidate |
| Google Benchmark | Performance harness | Apache-2.0; verify selected revision | Candidate |

GPL projects such as BYOD and Guitarix may be studied as references, but their source must not be copied into OpenRig without an explicit licensing decision and ADR.
