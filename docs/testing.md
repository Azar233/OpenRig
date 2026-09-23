# 测试策略与质量门禁

测试是实现的一部分，不是最后的清理工作。每条结果都必须能通过记录的命令、Fixture 和构建配置复现。

## 测试层级

| 层级 | 范围 | 必需证据 |
|---|---|---|
| Unit Test | DSP 基础算法、Node、Queue、Serializer | 确定性的 Pass/Fail 与数值容差 |
| Graph/Component Test | 校验、执行顺序、Buffer、参数、Swap、资源 | 正向与负向拓扑案例 |
| Integration Test | iPlug2 Audio Adapter、设备、NAM、IR、Preset、Plugin State | Host/设备/版本矩阵 |
| Golden Audio Test | 音色相关回归 | 输入 WAV、参考 WAV、算法/版本元数据、容差 |
| Realtime Stress Test | 分配、异常、内存安全、Graph Swap | Block 数、随机种子、Sanitizer/Guard 结果 |
| Benchmark | mean/p95/p99 与 Realtime Factor | 机器、工具链和构建配置元数据 |
| Soak/Release Test | 连续的端到端处理 | 时长、采样率/Block、dropout/crash/deadlock 计数 |

当前测试本身无第三方框架依赖，并已拆分为 `openrig_node_tests`、`openrig_graph_compiler_tests`、`openrig_graph_execution_tests`、`openrig_parameter_transport_tests`、`openrig_audio_engine_tests` 和 `openrig_iplug_adapter_tests` 六个独立 CTest。AudioEngine 测试包含 Graph Swap、Queue 回压、所有权与 100,000 Block 实时分配压力；Adapter 测试覆盖 float/double、mono/stereo、零输入、额外输出、可变及超大 Block。若以后引入 Catch2，必须固定版本，且不能阻塞当前门禁。

## 每个 Audio Node 必测的输入

每个 Node 至少处理以下输入：

- Silence；
- 在多个位置出现的单 Sample Impulse；
- 正、负 DC；
- 低于 Nyquist 频率时的 100 Hz、1 kHz 和 10 kHz Sine；
- 固定 Seed 的 Noise；
- 满刻度以及轻微超过满刻度的输入；
- 长度为 1、常用长度、非 2 的幂长度和 `maxBlockSize` 的 Block。

通用断言：不崩溃、不抛异常、输出有限、没有非预期 DC、`reset()` 后行为可复现、Bypass 正确、参数范围正确，且不写出声明的帧/声道区域。

## Node 专属断言

- Gain：乘法结果符合精确值或容差，smoothing 时长正确。
- Filter：选定频点上的幅频/相频响应与稳定性。
- Delay：Impulse 时序、Feedback 衰减、Mix 语义与 Tail。
- Dynamics：Detector 时序、Threshold/Ratio 曲线和静音恢复。
- Modulation：Rate/Depth 边界与声道布局。
- Nonlinear/Amp：输出有限、Alias/Oversampling 对比、DC 行为与电平边界。
- Convolution：Impulse 输出在容差内等于 IR，并核实 Latency 与 IR Swap。
- NAM：支持的采样率行为、Audio Thread 外 prewarm、可复现的模型切换，以及文件缺失/损坏错误。

## Graph 测试

正向案例：空 Graph 直通、单 Node、有序多 Node 链、Bypass、可变 Block、显式声明的 mono/stereo 转换，以及 Latency/Tail 汇总。

负向案例：保留或重复 ID、未知类型、缺失端点、重复边、自环、尚不支持的分支/汇合、环、不连通组件、不支持的声道布局、超过上限的 Block。

Graph Swap 压力测试需在 Control Producer 持续发布有效 Graph 并回收旧 Graph 的同时持续处理音频。验收要求：Audio Thread 无分配/加锁，无 use-after-free、内存泄漏或状态损坏，最终所有权可确定。

## Golden Audio 规则

- 只保存体积小、允许再分发的 Fixture，并记录 License 与来源。
- 参考文件元数据应包含 Node type/版本、采样率、Block 序列、参数状态、编译器与容差算法。
- 当浮点计算或编译器差异可预期时，应使用信号指标（最大绝对误差、RMS 误差、相关性、频谱包络），不要求字节完全相同。
- 主动修改音色算法时，更新参考音频必须附评审说明，解释听感和算法变化。

## 实时安全

每个 Node 都应在实时分配 Guard 下至少运行 100,000 个 Process Block。Core 压力测试需混合固定 Seed 的参数事件与 Graph Swap。目标：

- Audio Thread Heap Allocation 为 0；
- Exception 为 0；
- Lock/Wait 为 0；
- 可用 Sanitizer 构建中的无效内存访问与 Race 为 0；
- 除非测试专门注入非有限输入并定义恢复规则，否则不得产生非有限 Sample。

## 性能测试协议

统一场景：

| Sample Rate | Block | Deadline |
|---:|---:|---:|
| 48 kHz | 64 | 1.333 ms |
| 48 kHz | 128 | 2.667 ms |
| 48 kHz | 256 | 5.333 ms |
| 96 kHz | 128 | 1.333 ms |

记录 Warmup、迭代次数、mean、p95、p99、最大值、Realtime Factor（`Block Duration / Processing Duration`）、CPU、OS、电源模式、编译器、编译选项及 commit。默认效果器链在基准机上的长期处理耗时必须低于 Callback Deadline 的 50%；设计目标为低于 25%。若环境元数据不同，必须标注差异，不能直接比较 Benchmark。

## Release Candidate 门禁

Standalone 至少进行 48 kHz、128 Samples、30 分钟的 Soak Test；期间 Crash、Deadlock、Graph Corruption 和 Audio Thread Allocation 均为 0。需记录 Driver/Audio Interface、实际 Dropout、CPU 和 OS，因为设备与系统调度不完全由 DSP 决定。

ASIO 的具体操作步骤和证据字段见 [ASIO 真实设备 Smoke/Soak 测试](asio-smoke-test.md)。

v0.1 Release Candidate 前必须满足：

- 该里程碑的所有 P0 任务均标记 `DONE`；
- 从干净工作区执行 Debug/Release 构建与测试并通过；
- 第三方依赖和 License 登记保持最新；
- Preset 兼容性与损坏输入测试通过；
- Plugin 目标存在时，VST3 验证及 Host 矩阵通过；
- Performance 和 Soak 报告保存在 CI 的 `artifacts/` 或指定的 Release 存储位置（大文件不提交到仓库）。

## 测试案例记录模板

```text
ID:
关联需求/任务:
构建版本/commit:
环境:
前置条件/Fixture:
步骤:
预期:
实际:
结果: PASS | FAIL | BLOCKED
证据:
负责人/日期:
```
