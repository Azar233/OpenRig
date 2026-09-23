# OpenRig

OpenRig 是模块化、实时的吉他 DSP 平台。首个产品目标是 Windows x64 Standalone 应用；后续的 VST3/CLAP 与 WebAssembly 通过适配层共享同一套 C++20 DSP Core。

本仓库按照项目调研对话中确定的架构建立：

- `core/`：音频接口、Graph 编译、参数传输、实时原语，以及 Preset 和资源管理边界。
- `dsp/`：不依赖应用框架的信号处理 Node。
- `framework/iplug/`：iPlug2 Adapter 与 Standalone 入口；iPlug2 类型不得泄漏到 Core 或 DSP。
- `apps/`：各产品入口。
- `integrations/`：隔离 NAM 与 WDF 等第三方 API。
- `ui/`：不得直接持有或调用 `AudioNode*`。

## 当前状态

当前交付的是 M1 架构骨架，**不是**已发布的吉他效果器。它包含可构建的 Core/DSP、线性 Graph、`AudioEngine`、Gain/SoftClip、iPlug2 Audio Adapter、Windows Standalone 空壳和六组独立 CTest。默认 Standalone 只启用 DirectSound/MME；GPLv3 ASIO 构建已具备独立 Preset，默认 Graph、真实设备链路与产品 UI 仍未完成。

进度以 [`todolist.md`](todolist.md) 为准。架构与质量门禁见 [`docs/`](docs/README.md)。

## 构建

前提：CMake 3.24+，以及安装了“使用 C++ 的桌面开发”工作负载的 Visual Studio 2022。仓库内置 Preset 面向首个支持平台 Windows x64。

```powershell
cmake --preset dev
cmake --build --preset dev-debug
ctest --preset dev-debug
```

如需使用其他编译器或平台，请显式指定本机已有的 CMake generator：

```powershell
cmake -S . -B build/local -DOPENRIG_BUILD_TESTS=ON
cmake --build build/local --config Debug
ctest --test-dir build/local -C Debug --output-on-failure
```

iPlug2 Standalone 需递归 submodule，并使用独立 Preset：

```powershell
git submodule update --init --recursive third_party/iPlug2OOS
cmake --preset iplug2
cmake --build --preset iplug2-release
ctest --preset iplug2-release
```

构建 GPLv3 ASIO 版本：

```powershell
cmake --preset iplug2-asio
cmake --build --preset iplug2-asio-release
ctest --preset iplug2-asio-release
```

## 不可破坏的实时边界

1. `GraphDescription` 是可编辑的 Control Thread 状态；`CompiledGraph` 是已准备好的实时执行状态。
2. 参数变化不能触发 Graph 重编译。
3. 文件 I/O、解析、内存分配、NAM prewarm 和 IR 预处理不能出现在 `process()` 中。
4. Audio Thread 不分配内存、不加锁、不等待、不写磁盘日志、不析构重型对象，也不调用 UI。
5. 新增第三方依赖前，必须固定版本、记录 License；涉及架构时补充 ADR，并通过相关测试。

修改 Core 公共接口前，请先阅读 [工程规范](docs/engineering-standard.md)。

## License

OpenRig 按 [GNU GPLv3（GPL-3.0-only）](LICENSE) 开源。分发 Binary 时必须提供对应源码和构建所需内容；具体工程要求见 [许可与发布规范](docs/licensing.md)。第三方组件保留各自的 Copyright、License 和 Trademark 条款。
