# OpenRig 工程规范 v0.1

本文中的“必须”“不得”“应该”“可以”具有规范性含义。

## 语言与命名

- 代码必须使用 C++20，不依赖编译器扩展。
- 类型采用 `PascalCase`；函数和变量采用 `camelCase`；常量采用 `kPascalCase`；namespace 使用小写。
- 公共代码放在 `openrig` 及 `openrig::graph`、`openrig::dsp` 等更具体的 namespace 下。
- Core 的 Sample 类型为 `float`；第三方模块若使用 `double`，转换应留在 Adapter 中。
- `1.0F` 与 `-1.0F` 表示正、负满刻度。Core 不得隐式 normalization、limiting、auto-gain 或 clipping。

## 公共接口

- `NodeId` 为 `uint64_t`，`0` 保留不用。JSON 中必须存为固定宽度的十六进制字符串，不能存为 JSON Number。
- 公开 Node type 使用稳定的命名空间式字符串，例如 `openrig.drive.soft_clip`。
- 公开 Parameter key 发布后保持稳定。运行时 `ParameterIndex` 不属于持久化标识。
- `AudioBlockView` 是非拥有型 View，不得分配、扩容或释放内存。
- Node 必须接受长度可变、且不超过 `PrepareSpec::maxBlockSize` 的 Block。
- Audio Thread 调用的 `process()`、`reset()` 和参数操作必须是 `noexcept`。
- 允许每个 Block 在 Node 层进行一次 virtual dispatch；不得在逐 Sample 循环中调用 virtual function。

## 实时线程规则

Audio Callback 及其调用路径中禁止：

- `new`、`delete`、`malloc`、`free`、容器增长、字符串构造；
- mutex、condition_variable、future、sleep 或阻塞式 atomic 等待；
- 文件系统、Stream/File I/O、JSON 解析、NAM/IR 加载、网络 I/O；
- 磁盘/控制台日志及 UI 调用；
- Graph 校验或编译、Node 创建、重型对象析构。

Audio Thread 代码应具有已知的执行上界。实时路径中的任何失败都必须安全降级，例如输出静音、Bypass、拒绝事件或写入固定容量诊断队列；不得抛出异常。

## 所有权

- 默认使用 `std::unique_ptr<T>` 表示所有权。
- Raw Pointer 与 Reference 默认不拥有对象。
- 共享不可变资源可以使用 `std::shared_ptr<const T>`，但 Audio Thread 不得触发其最终析构。
- Graph/资源替换走 staged queue 与 retire queue；构造、析构都在 Audio Thread 外完成。

## Graph

- UI/Control 拥有 `GraphDescription`；Audio Thread 只持有已准备好的 `CompiledGraph`。
- 结构变化应编译新 Graph；Knob/Switch 参数变化不应重新编译。
- Compiler 必须拒绝重复或保留的 ID、缺失端点、未知 Node type、环、不连通输出、不支持的 Routing 或声道布局。
- v0.1 的 UI/Compiler 仅开放连通的线性 Graph，但内部 API 保持 Graph 形态。
- 所有 Node 存储、Scratch Audio 与处理计划必须在发布前准备完成。

## 参数

- UI、Preset、Host 与未来的 MIDI 都应生成 `ParameterEvent`。
- UI 代码不得持有 `AudioNode*`。
- 连续参数应该进行 smoothing。初始建议：Gain/Drive/Mix 为 5–20 ms，Frequency 为 10–50 ms。
- Boolean、Choice 与模式类参数在音频块边界更新，不做插值。
- Queue 溢出必须可观察并由调用方处理；不得静默阻塞 Producer。

## 资源与 Preset

- Node 接收已准备好的资源，不直接接收文件系统路径。
- NAM prewarm、WAV 解码、IR 重采样和卷积计划创建都在 Worker 上执行。
- Preset 必须包含 `schemaVersion` 和 `appVersion`。
- 未知 Node type 应转换成自动 Bypass 的 `MissingNode`，并保留原始 JSON 以支持 Round-trip。
- 外部资源使用 `ResourceRef`，不要把机器专属路径写死。查找顺序为 Preset 相对路径、用户资源库、最后是记录过的绝对路径。

## 错误与日志

- Control/Worker 边界内部可以使用异常，但第三方异常必须在状态发布前捕获。
- 对预期内的失败优先使用结构化 Result 或 Error Code。
- Audio Thread 的错误进入固定容量诊断队列，由非实时消费者格式化并持久化。
- 测试和 Debug 构建应该使用 `RealtimeScope` 与分配拦截 Hook，使实时分配直接导致测试失败；Hook 的实现由 T037 跟踪。

## 第三方依赖变更

- 固定版本或 commit，并更新 `THIRD_PARTY.md`。
- 以选定版本为准核实 License 与传递依赖义务。
- 架构级依赖变更需要 ADR。
- 未经明确的 License 决策，不得复制 GPL 参考项目的代码。

## 格式与 Include

- 头文件使用 `#pragma once`。
- 按需 Include；Project Header 放在 Standard Header 前。
- 除平台边界和编译时配置外，避免使用 Macro。
- Core 公共头文件不得包含生成文件或 Framework Header。
