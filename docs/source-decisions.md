# 来源对话：已采纳的决策

来源：[架构调研分享对话](https://chatgpt.com/share/6ab08468-328c-83ee-a532-86e82784e7fe)。

本文提炼仓库已经采纳的决策，不是对话逐字稿。对话中有关产品、Library、License 与平台支持的描述不应视为永久有效；集成依赖时必须重新核实。

## 产品目标

- 构建免费或低成本的模块化吉他综合效果器，而不是单一固定效果器。
- 先做 Desktop，再做 VST3/CLAP，最后考虑 Web/WASM。
- v0.1 的界面仅提供线性 Pedalboard，但底层模型应为后续 Graph 扩展做好准备。
- MVP 音频链包括 Input Gain、Noise Gate、Compressor、Overdrive、Amp、Cabinet IR、Chorus、Delay、Reverb、Output Gain、Tuner 和 Preset。
- 暂缓任意并行 Routing、Graph 反馈、VST Hosting、Cloud、用户账号、移动端、NAM Training 和 Marketplace。

## 技术基线

- C++20 与 CMake。
- iPlug2 仅位于 Platform/Plugin 边界，不进入 Core/DSP。
- 自研 DSP Core、Graph、参数和资源管理系统。
- NAM Core 封装在 `integrations/nam/`；WDF 封装在 `integrations/wdf/`。
- 内部音频格式为 planar float32。
- Preset 使用带版本号的 JSON，并使用稳定的字符串标识。
- 首个可运行产品目标为 Windows x64 Standalone。

## 架构不变量

- 可编辑 Graph 与实时执行 Graph 是两个独立对象。
- Graph 在音频块边界从 staged 状态切换；旧 Graph 在 Audio Thread 外回收。
- 参数通过 Parameter Engine 和 smoothing 更新；UI 不持有 DSP 对象指针。
- NAM、IR、Preset 资源在 Worker 上加载、解析、准备和 prewarm，再进入 staged 切换。
- Graph Scratch Buffer 在编译阶段分配。
- Core/DSP 无须 iPlug2 即可构建和测试。

## 里程碑

- M1：Core 宿主骨架
- M2：基础效果器
- M3：Amp 与 Cabinet
- M4：NAM
- M5：正式 UI
- M6：VST3/CLAP
- M7：Web/WASM
