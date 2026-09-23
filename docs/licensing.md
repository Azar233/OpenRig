# 许可与发布规范

## 项目许可

OpenRig 采用 **GNU General Public License Version 3（GPL-3.0-only）**。完整条款见仓库根目录的 `LICENSE`。

这表示：

- 可以使用、研究、修改和再分发 OpenRig，也可以收费提供副本或服务；
- 对外分发 OpenRig 或其修改版的 Binary 时，必须同时按 GPLv3 提供对应源码和构建所需脚本；
- 衍生作品不得附加限制接收者行使 GPLv3 权利的条款；
- 本项目没有授予 OpenRig、ASIO、Steinberg 等名称或 Logo 的 Trademark 权利。

公开 Git 仓库不能替代发布合规。Release 必须对应一个可复现的 Commit/Tag，并确保公开源码与实际 Binary 一致。

## ASIO

OpenRig 在 ASIO 路径中选择 Steinberg ASIO SDK 的 GPLv3 许可选项。构建必须使用固定在 `third_party/asio` 的 SDK，不得退回 iPlug2/RtAudio 内嵌但缺少当前许可证文件的旧快照。

ASIO 名称和 Logo 属于独立的 Trademark 范畴。当前项目不把 ASIO Logo 作为发布必需资产；若未来使用，必须先按固定 SDK 中的 `Steinberg ASIO Usage Guidelines.pdf` 完成检查。

## 贡献

提交贡献即表示贡献者有权提交该内容，并同意按 `GPL-3.0-only` 许可贡献。不得复制来源不明、仅供参考、禁止再分发或与 GPLv3 不兼容的代码、模型、IR、音频 Fixture 或图像。

外部 GPLv3 代码即使许可证兼容，也必须在合入前记录来源、精确版本、修改内容和 Copyright/License Notice；不能只凭“同为 GPL”直接复制。

## Release 检查清单

1. 根目录包含完整 `LICENSE`，README 明确标注 `GPL-3.0-only`。
2. Binary 对应的 Commit/Tag、完整源码、Submodule Commit 和构建步骤可取得。
3. `THIRD_PARTY.md` 与随包第三方 Copyright/License Notice 保持最新。
4. ASIO Build 只使用 `third_party/asio` 中固定的 GPLv3 SDK。
5. 若使用 ASIO Trademark 或 Logo，单独检查 Steinberg Usage Guidelines。
6. Release 证据记录编译器、配置、测试结果和产物校验值。

本文件是项目工程规范，不构成法律意见；发布方式或依赖发生重大变化时应重新审查。
