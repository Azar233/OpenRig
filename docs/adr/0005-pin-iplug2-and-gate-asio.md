# 0005 固定 iPlug2OOS，并为 ASIO 设置许可门

状态：已采纳

日期：2026-09-23

## 背景

M1 需要 Windows Standalone Host，但 Core/DSP 不得依赖宿主框架。iPlug2 默认 `sample` 为 double，OpenRig Core 固定使用 planar float32。iPlug2OOS 是官方推荐的 out-of-source 模板，并通过 gitlink 固定 iPlug2 Core。

固定版本内包含 RtAudio 的 Steinberg ASIO SDK 源文件。Steinberg 当前公开的 ASIO 开源许可路径是 GPLv3，也提供 proprietary 路径；OpenRig 尚未选择与之兼容的项目发布许可。

## 决策

- 顶层 submodule 固定 iPlug2OOS `9b214c1121a0da1e8db14b86a565c942a3bfea7e`，沿用其固定的 iPlug2 Core `b64192fe18afd9bc9a1fe324db5aceb48f4a0eee`。
- `OPENRIG_BUILD_IPLUG2` 默认关闭；关闭时 Core、DSP、Adapter 与 Test 不解析 iPlug2 CMake。
- `IPlugAudioAdapter` 不包含 iPlug2 Header，通过预分配 Buffer 完成 Host float/double 与 Core float32 的转换。
- `OpenRigStandalone::ProcessBlock()` 只调用 Adapter，Adapter 再调用 `AudioEngine::process()`。
- `OPENRIG_IPLUG2_ENABLE_ASIO` 默认关闭；关闭时从 `iPlug2::APP` 移除 ASIO 源码与宏，仅保留 DirectSound/MME。
- 开启 ASIO 或发布含 ASIO 的 Binary 前，必须记录 GPLv3-compatible 或 proprietary License 决策。

## 影响

固定依赖和 Standalone 可重复构建，且 Framework 类型不会污染 Core。Host double 转 float32 会增加一次输入/输出转换成本，后续需 Benchmark。ASIO 真实设备验收暂不能作为默认发布门禁，直到项目许可路径和硬件测试环境确定。
