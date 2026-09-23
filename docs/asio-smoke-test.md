# ASIO 真实设备 Smoke/Soak 测试

该文档用于 T003/T044 的人工或自托管 Runner 验收。自动 CTest 不能替代真实 Driver、Audio Interface 和系统调度测试。

## 安全前提

- 使用耳机或低音量监听，先关闭扬声器直通和可能产生 Feedback 的 Loopback。
- 确认乐器输入为 Instrument/Hi-Z（若设备支持），Output Gain 从最低位置开始。
- 关闭会独占同一设备的 DAW、浏览器通话和其他 Audio Application。
- 使用 `iplug2-asio-release` Build，并记录被测 Commit。

## 基础步骤

1. 执行 `build/iplug2-asio/out/OpenRigStandalone.exe`。
2. 在 Audio Preferences 中选择 ASIO Driver、mono Input 和 stereo Output。
3. 设置 48 kHz、请求 128 Samples；记录 Driver 返回的实际 Buffer Size。
4. 启动 Audio，确认输入不是 Silence，左右 Output 都收到同一条 mono Core 链结果。
5. 对比 Bypass/参考输入，确认运行的是 Gain → SoftClip → Gain，而不是 dry passthrough。
6. 修改 Input Gain、Drive、Output Gain；确认参数经过平滑且无明显 Zipper Noise。
7. 停止并重新启动设备，再切换一次 Sample Rate/Buffer 后恢复 48 kHz/128，确认无 Crash 或悬挂。
8. 连续运行 30 分钟，记录 Dropout/Underrun、Crash、Deadlock、异常静音和可闻 Glitch。

iPlug2 APP Host 当前以 `APP_SIGNAL_VECTOR_SIZE=64` 调用 Plugin，因此设备 Buffer 为 128 时通常会拆成两个 64-frame Core Callback。报告必须同时写“设备实际 Buffer”和“Core Processing Vector”，不能混为一项。

## 通过标准

- 真实输入经默认 Graph 到达左右 Output；
- 48 kHz/128 Samples 连续 30 分钟无 Crash、Deadlock 或 Graph Corruption；
- 检测到的 Audio Thread Allocation 为 0；
- 参数变化可听且平滑；
- 启停设备不在 Audio Thread 析构 Graph；
- Dropout/Underrun 必须如实记录；非零时不得写成“完全通过”，应附环境和复现条件。

## 记录模板

```text
测试日期：
OpenRig Commit：
Build 配置：
Windows 版本：
CPU / 电源模式：
Audio Interface：
Driver / 版本：
输入通道：
输出通道：
Sample Rate：
请求 / 实际设备 Buffer：
Core Processing Vector：
运行时长：
Dropout / Underrun：
Audio Thread Allocation：
参数测试结果：
启停 / 重配置结果：
其他观察：
结论：PASS / CONDITIONAL PASS / FAIL
```

当前开发机只读检测到 Realtek ASIO `3.1.10.8`。这只证明 Driver 已注册且 DLL 存在，不代表设备打开、通道路由或 30 分钟 Soak 已通过。
