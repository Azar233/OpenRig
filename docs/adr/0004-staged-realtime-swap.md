# 0004 分阶段切换 Graph 与资源

状态：已采纳

日期：2026-09-21

## 背景

Graph 编译、NAM 加载和 prewarm、IR 预处理以及对象析构都不适合在实时线程执行。Audio Callback 中使用 mutex 可能造成 dropout 或优先级反转。

## 决策

在 Audio Thread 之外准备替代状态，通过有界 SPSC mailbox 发布；仅在音频块边界切换；旧对象进入 retire queue，由非实时线程销毁。

## 影响

Audio Thread 不负责构造、加锁或重型析构。必须明确测试所有权、队列满载时的行为，并确保 Control Thread 定期回收旧对象。
