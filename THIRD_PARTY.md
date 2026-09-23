# 第三方依赖登记

当前骨架没有 vendor 或自动下载任何外部源码依赖。

引入依赖前，必须记录精确版本或 commit、上游 URL、License、用途、负责人、更新方法与再分发义务。Release 构建不得追踪未固定的 `main` 或 `latest`。

| 依赖 | 计划用途 | 待核实的 License | 状态 |
|---|---|---|---|
| iPlug2 / iPlug2OOS | Standalone、VST3、CLAP 与后续 WAM Adapter | zlib-like；以选定版本为准 | 计划中（T002） |
| NeuralAmpModelerCore | 神经网络箱头推理 | MIT；核实选定版本和传递依赖 | 计划中（T025） |
| Eigen | NAM 线性代数 | MPL2 及附带声明；核实配置 | 随 NAM 计划接入 |
| chowdsp_wdf | 电路/WDF 模型 | BSD-3-Clause；核实选定版本 | 计划中（T029） |
| nlohmann/json | Preset 序列化 | MIT；核实选定版本 | 计划中（T021） |
| Catch2 | Unit/Property Test | BSL-1.0；核实选定版本 | 候选 |
| Google Benchmark | 性能测试框架 | Apache-2.0；核实选定版本 | 候选 |

BYOD、Guitarix 等 GPL 项目可以作为研究参考；未经明确的 License 决策与 ADR，不得将其源码复制到 OpenRig。
