# 第三方依赖登记

当前仓库通过 Git submodule 固定 iPlug2OOS，并递归固定其 iPlug2 Core。默认 `dev`/`ci` Preset 不加载 iPlug2；只有 `OPENRIG_BUILD_IPLUG2=ON` 时才配置 Standalone 依赖。

引入依赖前，必须记录精确版本或 commit、上游 URL、License、用途、负责人、更新方法与再分发义务。Release 构建不得追踪未固定的 `main` 或 `latest`。

| 依赖 | 固定版本 | License / 义务 | 用途与状态 |
|---|---|---|---|
| [iPlug2OOS](https://github.com/iPlug2/iPlug2OOS) | `9b214c1121a0da1e8db14b86a565c942a3bfea7e` | 该固定 revision 未包含独立 `LICENSE` 文件；仅以官方 Template submodule 使用，Release 前仍需确认模板文件的授权声明 | Out-of-source CMake/CI 模板来源；已固定，T002 保持 `IN REVIEW` |
| [iPlug2](https://github.com/iPlug2/iPlug2) | `b64192fe18afd9bc9a1fe324db5aceb48f4a0eee`（由 iPlug2OOS 固定） | zlib-like；保留 `LICENSE.txt` 及第三方声明 | Standalone Host；已接入 |
| Cockos WDL | 随 iPlug2 Core revision | zlib-like；保留源文件声明 | iPlug2 Core 传递源码 |
| RtAudio | 随 iPlug2 Core revision | MIT-like；再分发时保留版权及许可文本 | Windows DirectSound/MME Host 后端 |
| RtMidi | 随 iPlug2 Core revision | MIT-like；再分发时保留版权及许可文本 | Windows MIDI Host 后端 |
| Microsoft WIL | `v1.0.240803.1`（由固定 iPlug2 CMake 声明） | MIT | iPlug2 配置期传递依赖；当前 `UI NONE` Target 不链接使用 |
| Microsoft WebView2 | `1.0.2903.40`（由固定 iPlug2 CMake 声明） | 随 NuGet 包的 `LICENSE.txt`/`NOTICE.txt` | iPlug2 配置期下载；当前 `UI NONE` Target 不链接使用 |
| Steinberg ASIO SDK | 固定 iPlug2/RtAudio 中随附版本 | [Steinberg 当前开源路径为 GPLv3](https://www.steinberg.net/developers/asiosdk-open/)，另有 proprietary 路径；必须先作项目 License 决策 | 默认通过 `OPENRIG_IPLUG2_ENABLE_ASIO=OFF` 排除源码和宏；T003 前处理 |
| NeuralAmpModelerCore | 待定 | MIT；核实选定版本和传递依赖 | 计划中（T025） |
| Eigen | 待定 | MPL2 及附带声明；核实配置 | 随 NAM 计划接入 |
| chowdsp_wdf | 待定 | BSD-3-Clause；核实选定版本 | 计划中（T029） |
| nlohmann/json | 待定 | MIT；核实选定版本 | 计划中（T021） |
| Catch2 | 待定 | BSL-1.0；核实选定版本 | 候选 |
| Google Benchmark | 待定 | Apache-2.0；核实选定版本 | 候选 |

## iPlug2 获取与更新

```powershell
git clone --recurse-submodules https://github.com/Azar233/OpenRig.git
git submodule update --init --recursive third_party/iPlug2OOS
```

更新必须同时记录新的 iPlug2OOS commit、其 `iPlug2` gitlink commit、License 差异和 Debug/Release 构建证据；不得只把 submodule 切回浮动的 `master`。

BYOD、Guitarix 等 GPL 项目可以作为研究参考；未经明确的 License 决策与 ADR，不得将其源码复制到 OpenRig。
