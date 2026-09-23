# 0006 采用 GPLv3，并固定当前 ASIO SDK

状态：已采纳

日期：2026-09-23

取代：ADR-0005 中“尚未选择项目许可，因此禁止开启 ASIO”的临时门禁；ADR-0005 的依赖分层和默认非 ASIO 构建决策继续有效。

## 背景

OpenRig 需要在 Windows Standalone 中验证低延迟 ASIO 链路。仓库原先随固定 iPlug2/RtAudio 带有一份 ASIO 2.3 旧快照，但该快照缺少 Steinberg 2025 年加入的顶层双许可文本，不能仅凭项目改为 GPLv3 就将它视为已完成许可审计。

Steinberg 当前公开的 ASIO SDK 同时提供 Proprietary 与 GPLv3 路径。项目已决定采用完整开源路线。

## 决策

- OpenRig 采用 `GPL-3.0-only`，根目录保存 GNU GPLv3 完整文本。
- 固定 `audiosdk/asio` Commit `496a0765b8bb9c26f764f22f9a9712a937177db2`，按其 GPLv3 选项使用。
- ASIO Build 从 iPlug2 的 `iPlug2::APP` 移除旧 `asio.cpp`、`asiodrivers.cpp`、`asiolist.cpp`，改为编译固定 SDK 中带当前许可证指向的对应文件。
- 保留 RtAudio 的 Public Domain `IASIOThiscallResolver` 兼容层；MSVC x64 下该层为空实现。
- 默认 `iplug2` Preset 仍关闭 ASIO；新增 `iplug2-asio` Preset，显式构建 GPLv3 ASIO 版本。
- 使用 ASIO Trademark 或 Logo 不是 GPLv3 授权的组成部分，必须另行遵守 SDK 中的 Usage Guidelines。

## 影响

OpenRig 的公开分发必须满足 GPLv3 的对应源码和再分发要求，不能再发布闭源的 OpenRig 衍生 Binary。ASIO 依赖现在具有固定 Commit 和可审计的许可证文本，但真实设备兼容性、通道选择与 30 分钟 Soak 仍需在 T003/T044 完成。
