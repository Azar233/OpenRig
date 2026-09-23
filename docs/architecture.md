# 系统架构

## 分层模型

```text
apps（standalone/plugin/web）
        |
framework adapters + ui
        |
core（engine/graph/parameters/resources/presets/realtime）
        |
dsp（与 framework 无关的 nodes）
        |
integrations（NAM/WDF）及固定版本的基础库
```

依赖只能向下。`core/` 和 `dsp/` 必须在不依赖 iPlug2、GUI、Host SDK 或应用层文件系统代码的情况下编译。`integrations/` 负责将第三方 API 适配为 Core 接口；第三方类型不得穿过公共边界。

## 仓库目录

```text
apps/
  standalone/          临时 DSP sandbox；未来接入 iPlug2 宿主
  plugin/              未来的 VST3/CLAP 入口
framework/
  iplug/               未来的 Audio/Plugin Adapter
core/
  include/openrig/core/
    audio/              Buffer、Prepare/Process 接口
    graph/              Description、校验、编译与执行
    parameter/          Descriptor 与事件传输
    preset/             未来的 Schema 映射与迁移
    realtime/           有界无锁原语与 Guard
    resource/           异步加载边界
dsp/
  include/openrig/dsp/  可复用的 DSP Node 与基础算法
integrations/
  nam/                  未来的 NAM Adapter
  wdf/                  未来的 chowdsp_wdf Adapter
ui/                     未来的展示层
tests/                  确定性测试
benchmarks/             实时预算与性能回归
docs/adr/               架构决策记录
third_party/            如需 vendor 的固定版本依赖
```

未来目录在出现首个归属文件时再加入，不必为了展示结构而提前创建空目录。

## 运行时线程与所有权

| 所在线程 | 持有对象 | 允许操作 | 禁止操作 |
|---|---|---|---|
| Audio Thread | 当前 `CompiledGraph`、预分配 Buffer、实时参数消费端 | 有界 DSP 与 atomic/lock-free 交换 | 分配、加锁、等待、文件/网络 I/O、解析、写日志、绘制 UI、析构重型对象 |
| Control/UI Thread | `GraphDescription`、用户编辑状态和命令状态 | 分配、校验、请求编译、推送参数事件 | 直接修改当前运行的 Graph/Node |
| Worker Thread | NAM/IR 解码、FFT Plan、准备和 prewarm | 文件 I/O、解析与高成本构造 | 进入实时处理路径 |
| Background/IO Thread | Preset 存储、资源扫描、非实时日志 | 文件系统操作与索引 | 持有 Audio Callback 状态 |

## Graph 生命周期

```text
GraphDescription（Control）
        |
校验：ID/type/端点/声道布局/环
        |
拓扑排序 + 创建 Node + prepare
        |
分配 Buffer + 生成处理计划
        v
CompiledGraph B
        |
SPSC publish mailbox
        v（音频块边界）
active A -> active B -> retire A
                         |
                    SPSC retire queue
                         |
                 Control Thread 销毁
```

v0.1 只接受一条连通的线性链。数据类型仍保留 Node 与 Connection，以便将来增加并行 DAG Routing 时不必改变 Preset 身份。Graph 层反馈无效；Delay/Reverb 可以在 Node 内部维护有界反馈。

当前 `GraphCompiler` 已完成线性链的声道能力校验、持久化参数应用和结构化错误码；`CompiledGraph` 预建 `NodeId` 参数路由，并对非法 Buffer/Block 返回状态且安全清零。`AudioEngine` 在 Block 边界从 SPSC mailbox 接收 Graph，将旧 Graph 放入 Retire Queue；Queue 满时暂存 Deferred Retire，绝不在 Audio Thread 析构。只有正式支持并行 Routing 时才需要做 DAG Buffer 生命周期分析。

## 参数生命周期

```text
UI / Preset / Host Macro / 未来的 MIDI
        |
ParameterEvent { NodeId, ParameterIndex, value }
        |
有界 SPSC queue
        |
音频块边界
        |
Node setParameter -> smoother/离散值锁存 -> process
```

运行时 Index 允许随版本变化；持久化必须使用稳定的 Node type 字符串和 Parameter key。连续参数定义 smoothing；Boolean、Choice 与模式类整数在音频块边界更新。

## 资源生命周期

Node 不得自行打开文件。`ResourceManager` 接收请求，Worker 加载并验证资源、准备和 prewarm 不可变对象，然后发布到音频块边界进行切换。最终析构必须发生在非实时线程；共享不可变资源的最后一次引用释放也不例外。

## Buffer 与声道约定

- 内部使用非交错的 planar float32 PCM。
- Callback 的实际帧数可变化，但必须 `<= maxBlockSize`。
- v0.1 吉他输入为 mono；Node 明确声明 mono-to-mono、mono-to-stereo 或 stereo-to-stereo；产品输出为 stereo。
- Compiler 拥有 Routing 与内存；Node 只接收非拥有型 View，不得调整 Buffer 容量。
- 线性链使用两个预分配的 ping-pong Scratch Buffer。In-place 处理能力必须显式声明，不能默认假设。

## 平台适配层

当前 iPlug2 调用路径：

```text
iPlug2 ProcessBlock
  -> IPlugAudioAdapter
  -> AudioEngine::process
  -> active CompiledGraph
```

v0.1 对 Host 暴露的参数限定为 Input/Output Gain 和固定数量的 Macro。动态内部 Node 不应迫使 Core 承担不稳定的 VST 动态参数身份。

`IPlugAudioAdapter` 只依赖 Core，并使用预分配 planar float32 Buffer 隔离 iPlug2 默认的 double `sample`。iPlug2 类型只存在于 `framework/iplug/standalone` 入口；`core/`、`dsp/` 和 Adapter 公共接口均不包含 iPlug2 Header。
