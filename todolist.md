# OpenRig 开发进度表

最后更新：2026-09-23

当前里程碑：**M1 — 实时宿主骨架**

进度规则：只有 `DONE` 计为完成。任务须同时具备代码、规定测试、文档和验收证据。

状态值：`TODO` · `IN PROGRESS` · `BLOCKED` · `IN REVIEW` · `DONE`

## M1 关键路径：五个工作包

这五项是本地规划工作包（W1–W5），**不是已创建的 GitHub Issue 编号**。依据是[补充架构讨论](https://chatgpt.com/share/6ab13d44-0ef8-83e9-bd93-0709e2c383eb)及仓库当前代码；具体实施仍以任务 ID、验收标准和测试证据为准。

| 工作包 | 内容与对应任务 | 前置条件 | 完成标志 |
|---|---|---|---|
| W1：Graph/参数闭环（已完成） | T005–T011、T013；已完成参数路由、声道能力、结构化错误和安全失败路径 | T004 已完成 | Gain → SoftClip → Gain 已通过正负向与可变 Block 测试，无需 iPlug2 |
| W2：AudioEngine/实时生命周期（已完成） | T042、T012、T037a；已建立框架无关的 Callback 边界、Graph Publish/Retire 与有界参数消费 | W1 的接口和基础执行闭环 | 100,000 Block 压力通过；Audio Thread 检测到的分配和 Graph 析构均为 0 |
| W3：iPlug2 依赖与适配 | T002 先固定版本并生成 Standalone 空壳；T043 再将 Adapter 接到 AudioEngine | T002 可与 W1 并行；T043 依赖 T042 | 依赖可复现，且关闭 iPlug2 时 Core/DSP/Test 仍可独立构建 |
| W4：真实设备链路 | T003；设备输入 → Adapter → AudioEngine → 默认 Graph → 设备输出 | W1、W2、W3 汇合 | 48 kHz/128 Samples 下可弹奏并记录设备配置及 Dropout |
| W5：测试与验收 | T041 可尽早启动 CI；T044 汇总独立 CTest、压力测试和真实设备 Soak | 自动测试可随 W1/W2 开始；最终验收依赖 W4 | Debug/Release CI 通过，真实设备完成 30 分钟 Smoke Soak |

W1、W2 已完成，下一步进入 W3 的 T002/T043，再与 W4 汇合。CI 不必等真实声卡接入才开始。T021a 最小 Preset Round-trip **不阻塞 W1–W5 的设备关键路径**，但仍是完整 M1 的完成条件。

### 采纳边界与风险

- [iPlug2 官方 CMake 文档](https://iplug2.github.io/docs/md_cmake.html)列出 Windows Standalone App Target，[iPlug2OOS](https://github.com/iPlug2/iPlug2OOS)提供 Out-of-source 项目模板，因此 W3 的构建方向可行；但尚未为本项目选定或验证具体 commit。
- iPlug2 的 [ASIO 非首输入通道选择 Issue #1281](https://github.com/iPlug2/iPlug2/issues/1281)目前仍为 open。T002/T003 必须用选定版本和实际多通道设备复测；若有缺陷，需决定修复 Adapter、贡献上游修复或采用其他设备方案，不能把“任意通道可选”视为现成能力。
- W1 的运行时参数路由应在编译阶段预建索引；Audio Thread 不得为查找而扩容或构造字符串。非法 Block 的静音/拒绝策略必须先验证 Buffer View 边界，避免“错误恢复”本身越界。
- W2 的 SPSC 设计必须明确单 Producer/单 Consumer、Queue 满载与旧 Graph 回收策略。Audio Thread 不能因 Retire Queue 满而析构旧 Graph；多来源参数写入也不能未经仲裁直接共用一个 SPSC Producer。
- 100,000 Block 自动压力测试只能证明所覆盖路径；“零锁”“零 Race”还需代码审计和适用的检测工具。真实 ASIO 的 30 分钟 Soak 属于手动或自托管 Runner 门禁，不应伪装成 GitHub 托管 CI 的硬件测试。

## M1 — 实时宿主骨架

| ID | 优先级 | 状态 | 任务 | 验收标准 | 依赖 | 备注 / 证据 |
|---|---:|---|---|---|---|---|
| T001 | P0 | DONE | 仓库与 CMake 骨架 | Debug/Release 可配置；具备 Core/DSP/Sandbox/Test Target；构建步骤有文档 | — | VS2022 x64 Debug、Release 及 warnings-as-errors 构建通过；当前 Debug/Release CTest 均为 5/5；Sandbox 输出 `0.7` |
| T002 | P0 | TODO | 固定 iPlug2OOS 并生成 Standalone 空壳 | iPlug2OOS 模板和 iPlug2 Core 均固定精确 commit，登记 License/传递依赖；新增 `OPENRIG_BUILD_IPLUG2` 开关；Windows x64 Debug/Release 可构建；关闭开关时 Core/DSP/Test 仍独立构建 | T001 | 属于 W3 可并行部分；不在此任务中接入完整 AudioEngine。复测官方 ASIO 通道选择风险 |
| T003 | P0 | TODO | 真实设备运行默认 Graph | 可选 ASIO 设备及已验证的输入/输出通道；mono 输入经过 Gain → SoftClip → Gain 到 stereo 输出；参数实时修改；启停和失败安全 | T010, T011, T012, T043 | W4；记录声卡、驱动、Sample Rate、请求/实际 Block、通道、时长及 Dropout；任意通道支持须实测 |
| T004 | P0 | DONE | 独立的 `openrig_core` Library | 无须 iPlug2/UI 即可构建与测试 | T001 | `openrig_core.lib` 在 Debug/Release 均构建通过，且不依赖 iPlug2/UI |
| T005 | P0 | DONE | `AudioNode` 接口与 Gain Node | Gain 支持可变 Block；Bypass/Reset/Descriptor 测试通过 | T004 | `openrig_node_tests` 覆盖 Descriptor、Reset、Bypass 与 10 ms 平滑；Graph 执行测试覆盖可变 Block |
| T006 | P0 | IN REVIEW | Buffer/Process 接口 | Planar View 不拥有内存；Process 无分配；边界有文档 | T004 | Planar View 和非法 Buffer/Block 安全边界已落地；自动分配检测仍由 T037a 补齐 |
| T007 | P0 | DONE | 参数事件引擎 | `NodeId + ParameterIndex` 更新正确 Node 且不重编译 Graph；Queue 溢出可观察；每 Block 消费有固定上限 | T005, T011 | `drainUpTo()` 提供硬上限；测试覆盖 FIFO、剩余事件、零预算、Queue 满载回压及 CompiledGraph 路由 |
| T008 | P0 | DONE | 参数平滑 | Gain/Drive 变化按 Descriptor 时长平滑；快速变化不产生明显 Zipper Noise；时长测试通过 | T007 | Gain 与 SoftClip Drive 的确定性 10 ms 时长测试通过 |
| T009 | P0 | DONE | `GraphDescription` | 稳定的 Node/Connection 描述可持久化线性 Graph | T004 | Node/Connection 描述已被编译和执行测试覆盖；Serializer 属于 T021 |
| T010 | P0 | DONE | `GraphCompiler` 完整线性校验 | 有效链可编译；保留/重复 ID、未知类型、缺失端点、重复边、自环、Branch/Merge/Cycle、不连通及不支持的声道布局有可断言错误码 | T009, T013 | 已增加 `GraphCompileErrorCode`、Capabilities 接线和独立负向测试，Debug/Release 通过 |
| T011 | P0 | DONE | `CompiledGraph` 执行与参数路由 | 编译时预建 `NodeId` 路由并应用初始参数；`applyParameter() noexcept` 无分配；可变 Block、Bypass、Latency/Tail、空图、单节点、mono→mono、显式 mono→stereo 及非法 Block 安全行为通过测试 | T010 | 预建排序路由、初始参数、声道转换、聚合指标及越界清零均有独立执行测试 |
| T012 | P0 | DONE | 分阶段 Graph Swap | Block 边界 SPSC Publish + Retire；Queue 满载时不阻塞、不在 Audio Thread 析构；重复切换与所有权测试通过 | T042 | SPSC Publish/Retire 已实现；满载时保留 deferred retire 并暴露计数，Control Thread drain 后恢复切换；析构线程测试通过 |
| T013 | P0 | DONE | Node Registry/Factory | 稳定 Type 创建 Node；重复/未知 Type 有明确结果；暴露供 Compiler 使用的声道能力 | T004 | 重复/未知 Type 和空 Factory 结果均有测试；Gain/SoftClip 显式声明 Capabilities，Compiler 已校验 |
| T021a | P0 | TODO | M1 最小 Preset Round-trip | 使用 Schema v1 保存并恢复简单 Gain/SoftClip Graph | T009, T013 | T021 的 M1 最小切片；可在设备闭环后完成，不阻塞 W1–W5 |
| T037a | P0 | DONE | 实时分配 Guard | 测试构建能捕获 `process()` 内的 `new`/容器增长；100,000 Block 参数与 Graph Swap 压力通过 | T042, T012 | 测试专用全局分配 Hook 已用主动 `new` 校准；100,000 Block 参数/Swap 压力检测到 0 次实时分配、0 次实时 Graph 析构；零锁和 Race 仍按完整 T037 审计 |
| T041 | P0 | TODO | M1 Windows CI | 从 clean checkout 完成 Debug/Release 构建并运行当前 CTest；保留日志和产物；T044 新增的测试应自动纳入 | T001 | 可立即与 W1/W3 并行，不等待设备；硬件 Soak 不放在 GitHub 托管 Runner |
| T042 | P0 | DONE | `AudioEngine::process()` 边界 | Core 内建立框架无关的处理入口；持有 active Graph；每 Block 有界消费参数；无 Graph 时安全直通或静音；Callback 入口使用 `RealtimeScope` | T007, T011 | 已实现框架无关入口、无 Graph 同布局直通/异布局静音、默认每 Block 64 个参数预算及 RealtimeScope；Adapter 只需单向调用 |
| T043 | P0 | TODO | iPlug2 Audio Adapter | `ProcessBlock` 单向调用 `AudioEngine::process()`；覆盖 mono/stereo、零输入、可变/超大 Block 和 Sample 格式边界；Core/DSP 不含 iPlug2 类型 | T002, T042 | W3 汇合部分；需验证选定版本的实际 Callback/设备行为 |
| T044 | P0 | IN PROGRESS | M1 集成与设备验收 | 拆分 Node/Graph/参数/Swap/Adapter 为独立 CTest；自动 100,000 Block 压力；真实 ASIO 48 kHz/128、30 分钟 Soak 并记录时序与 Dropout | T003, T012, T037a, T041 | 已拆分 Node、Graph Compiler、Graph Execution、Parameter Transport、AudioEngine 五组 CTest，并完成 100,000 Block 压力；Adapter 与硬件验收待后续任务 |

### M1 完成门槛

W4 完成表示“可弹奏的技术原型”，W5 完成表示“经过基本可靠性验证的原型”；二者均不自动等于完整 M1 `DONE`。

- Standalone 的 Audio Passthrough 在 48 kHz/128 Samples 下工作。
- Gain → SoftClip → Gain Graph 可执行，参数可实时平滑调整。
- 添加、删除、重排 Node 时可重新编译与切换 Graph，Audio Thread 不加锁、不分配、不析构。
- 最小 Preset Round-trip 成功。
- 独立 CTest、负向/实时压力测试，以及真实设备 30 分钟 Soak Test 均通过；失败能定位到具体测试。

## M2 — 基础效果器与可用的 Pedalboard

| ID | 优先级 | 状态 | 任务 | 验收标准 | 依赖 |
|---|---:|---|---|---|---|
| T014 | P1 | TODO | Biquad Filter | LPF/HPF 响应与稳定性测试通过 | M1 |
| T015 | P1 | TODO | Noise Gate | Threshold/Attack/Release 测试通过；输出有限 | T014 |
| T016 | P1 | TODO | Compressor | Attack/Release/Threshold/Ratio 行为通过测试 | T014 |
| T017 | P1 | TODO | 产品级 SoftClip Overdrive Node | Drive/Tone/Level 与 Golden Audio 测试通过 | T014, T008 |
| T018 | P1 | TODO | Delay | Time/Feedback/Mix，以及 Impulse/Tail 测试通过 | M1 |
| T019 | P1 | TODO | Chorus | Rate/Depth/Mix 正确；明确声明 mono-to-stereo 能力 | T018 |
| T020 | P1 | TODO | Reverb | 基础 Room/Hall 可用；稳定性、Tail、性能测试通过 | M1 |
| T031 | P0 | TODO | 基础 Pedalboard UI | 支持添加/删除/Bypass/重排，不直接持有 Node 指针 | T002, T012 |
| T032 | P0 | TODO | Descriptor 驱动的参数面板 | 根据 Descriptor 生成 Knob/Switch/Choice；只通过 Queue 更新 | T007, T031 |
| T033 | P1 | TODO | Input/Output Meter | Audio 到 UI 的 Telemetry 有界；Audio Thread 不直接绘图 | T003 |
| T034 | P1 | TODO | Preset Browser | 保存/读取/错误/资源缺失的 UX 完整 | T021 |

## M3 — Amp 与 Cabinet

| ID | 优先级 | 状态 | 任务 | 验收标准 | 依赖 |
|---|---:|---|---|---|---|
| T021 | P0 | TODO | 完整 Preset Schema v1 | 带版本的 Graph/Resource；未知 Node Round-trip；迁移测试通过 | T021a, M2 |
| T022 | P1 | TODO | IR Loader | 支持的 WAV 变体通过校验；损坏/超大输入可控 | T027 |
| T023 | P1 | TODO | Partitioned Convolver | Impulse/Golden/Latency/Performance 测试通过 | T022 |
| T024 | P1 | TODO | Cabinet Node | IR + HPF/LPF；资源分阶段切换 | T014, T023 |
| T027 | P0 | TODO | 异步 ResourceManager | I/O/Parse/Prepare/Prewarm 不在 Audio Thread；错误可观察 | T012 |
| T028 | P1 | TODO | Procedural Amp | Clean/Crunch，包含 Oversampling/Alias Benchmark | T014, T017 |
| T029 | P1 | TODO | 接入 chowdsp_wdf | 固定版本并记录 License；RC/Diode Demo 通过测试 | T001 |
| T030 | P2 | TODO | WDF Overdrive | 首个电路级效果器通过 Golden Audio 与 Benchmark | T029 |

## M4 — NAM

| ID | 优先级 | 状态 | 任务 | 验收标准 | 依赖 |
|---|---:|---|---|---|---|
| T025 | P0 | TODO | 接入 NAM Core | 记录固定版本、License 和传递依赖；`.nam` 在 Audio Thread 外加载 | T027 |
| T026 | P0 | TODO | `NamNode` | 成为普通 Graph Node；Sample Rate/Prewarm/Swap/Latency 测试通过 | T025 |

## M5/M6 — 正式 UI 与 Plugin

| ID | 优先级 | 状态 | 任务 | 验收标准 | 依赖 |
|---|---:|---|---|---|---|
| T035 | P0 | TODO | VST3 Build | 完成验证，并在约定 DAW 矩阵中成功加载 | M2, T002 |
| T036 | P0 | TODO | Plugin State Restore | 重开 DAW 后 Graph/参数/资源恢复一致 | T021, T035 |
| T037 | P0 | TODO | 完整实时安全审计 | Callback 分配与锁均为 0；提供 Sanitizer/压力测试报告 | M4 |
| T038 | P0 | TODO | Benchmark Suite | Node 与默认链场景含 mean/p95/p99/RTF 数据 | M2 |
| T039 | P1 | TODO | 完整 CI Matrix | Windows 构建/测试/打包；目标建立后加入 macOS | T041, T035 |

## M7 — Web/WASM

| ID | 优先级 | 状态 | 任务 | 验收标准 | 依赖 |
|---|---:|---|---|---|---|
| T040 | P2 | TODO | WAM/Web Prototype | 同一 DSP Core 经 WASM + AudioWorklet 运行；记录延迟 | M6 |

## Backlog 规则

- M1 完成门槛满足之前，不批量推进 M2 效果器。
- 不为解除 M1 阻塞而提前加入 NAM、精美正式 UI 或 Procedural Amp。
- 新工作在编码前必须获得 ID、优先级、依赖和可测量的验收标准。
- 遇到阻塞时，在备注中记录具体原因与下一步决策或行动；`BLOCKED` 不能代替对任务的澄清。
