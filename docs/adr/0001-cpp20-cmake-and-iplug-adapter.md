# 0001 使用 C++20/CMake，并将 iPlug2 限定为适配层

状态：已采纳

日期：2026-09-21

## 背景

OpenRig 需要支持 Standalone、Plugin，以及后续的 Web 入口，同时避免重复实现 DSP。Framework 的许可和长期可移植性也必须考虑。

## 决策

使用 C++20 和 CMake。iPlug2/iPlug2OOS 仅放在 `framework/iplug`、应用目标和 UI 中。Core/DSP 的公共头文件不得包含 iPlug2。

## 影响

Core 测试无需依赖 Plugin Framework；新宿主可以复用同一套 DSP。适配层必须明确转换 Audio Buffer、参数、状态、延迟和 UI 消息。
