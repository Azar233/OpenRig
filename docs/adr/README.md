# 架构决策记录（ADR）

ADR 是不可随意改写的决策历史。决策改变时，应新增 ADR 并注明它取代了哪一份旧记录；旧记录仅允许修正笔误或澄清措辞。

当前记录：

- `0001`：C++20、CMake 与 iPlug Adapter 分层；
- `0002`：Graph-ready 数据模型，v0.1 只开放线性链；
- `0003`：Core 使用 planar float32；
- `0004`：Graph/资源 staged swap 与非实时回收；
- `0005`：固定 iPlug2OOS/iPlug2，并在许可决策前默认关闭 ASIO；
- `0006`：项目采用 GPLv3，固定当前双许可 ASIO SDK，并新增显式 ASIO 构建路径。

模板：

```markdown
# NNNN 标题

状态：提议中 | 已采纳 | 已被取代
日期：YYYY-MM-DD

## 背景

## 决策

## 影响
```
