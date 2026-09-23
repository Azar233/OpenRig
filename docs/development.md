# 开发流程

## 工具链

- CMake 3.24 或更高版本
- C++20 编译器：当前 MSVC 是首个支持的工具链；Clang/GCC 为可移植性目标
- 仓库内置 Preset 使用 Visual Studio 2022；其他平台可显式选择本机 CMake generator
- Git

当前骨架不会自动下载 iPlug2、NAM、WDF 或 JSON 依赖。应在对应任务中固定版本、完成 License 审查后再接入。

## 任务流程

1. 在 `todolist.md` 中选择一个未被阻塞的任务。
2. 标记为 `IN PROGRESS`，记录负责人和日期。若需要修改验收标准，先记录范围决策。
3. 改动架构边界前新增或修订 ADR。
4. 实现满足验收标准的最小垂直功能切片。
5. 运行规定的测试，在任务的“备注/证据”中记录结果。
6. 标记为 `IN REVIEW`；评审和全部检查通过后，标记为 `DONE`。

合法状态为 `TODO`、`IN PROGRESS`、`BLOCKED`、`IN REVIEW` 和 `DONE`。不得凭印象填写完成百分比；里程碑进度按“`DONE` 任务数 / 总任务数”计算，并单独列出 P0 阻塞项。

## Branch 与 Commit 约定

建议使用 `feat/T007-parameter-engine`、`fix/T010-cycle-detection`、`docs/ADR-0005-resource-lifetime` 等 Branch 名。

Commit 标题应使用祈使语气并明确范围。格式化、依赖升级和行为修改不要混在同一个 Commit。

## 新增 Audio Node

1. 选定稳定的命名空间式 Node type 和 Parameter key。
2. 定义 Descriptor、声道能力、Latency/Tail 行为及默认 Bypass 行为。
3. 在 `prepare()` 中分配全部状态，并支持可变的 Callback Block 长度。
4. 实现有界的 `noexcept process()` 和明确的 `reset()`。
5. 通过 `NodeRegistry` 注册；不要添加不断膨胀的类型判断链。
6. 补齐 `testing.md` 要求的测试；非平凡 Node 还需 Benchmark，最后更新任务表证据。

## 修改 Graph 或 Preset Schema

这类改动必须有 ADR、迁移与兼容方案、负向测试，并更新 `docs/preset-schema.md`。一旦发布，Node type 和 Parameter key 就属于兼容性契约。

## 本地验证

```powershell
cmake --preset dev
cmake --build --preset dev-debug
ctest --preset dev-debug
```

涉及 Release 音频行为的变更还应运行 Release 测试与 `docs/testing.md` 中规定的 Benchmark。
