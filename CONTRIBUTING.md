# 参与开发

## 开始编码前

1. 阅读 `docs/architecture.md`、`docs/engineering-standard.md` 和 `docs/testing.md`。
2. 从 `todolist.md` 选择一个未被阻塞的任务；实施前标记为 `IN PROGRESS`，记录负责人和日期。
3. 若变更 Graph 模型、`AudioNode` ABI、参数模型、所有权、Preset Schema 或 Plugin 参数策略，先新增 ADR。

## 变更要求

- 提交贡献即表示你有权提交该内容，并同意按项目的 `GPL-3.0-only` 许可发布；详见 `docs/licensing.md`。
- 每次提交保持主题集中；Commit 标题使用祈使语气，例如 `Add linear graph validation`。
- 行为变更必须同步新增或更新测试。
- 执行配置、构建、CTest 及任务专属检查。
- 在 `todolist.md` 中更新状态，并填写测试名称、Benchmark 路径或产物等验收证据。
- 不得提交构建产物、受版权限制的模型、商业 IR、凭据或本机专属绝对路径。

## 评审检查项

- 依赖方向是否保持正确。
- `process()` 是否仍为 `noexcept`、有界、无分配且无锁。
- 公开的 Preset key 和 Node type 字符串是否稳定。
- Buffer 长度是否按 `PrepareSpec::maxBlockSize` 校验。
- 声道布局、延迟和 Tail 行为是否明确。
- 错误是否在非实时边界处理。
- 测试是否满足 `docs/testing.md`。

## 完成定义（Definition of Done）

只有代码、必需测试、文档和验收证据都齐备，任务才能标记 `DONE`。仅能编译但缺少规定测试的任务应保持 `IN REVIEW` 或 `BLOCKED`。
