# 0003 Core 使用 float32 planar 音频格式

状态：已采纳

日期：2026-09-21

## 背景

实时吉他 DSP、SIMD 和 NAM 集成需要可预测的 Buffer 布局与内存带宽。部分外部 API 可能使用其他数据格式。

## 决策

Core 内部统一使用非交错的 planar 32-bit float PCM。交错格式或精度转换由 Adapter 负责。

## 影响

所有 Node 共享高效、统一的处理接口。格式转换开销被明确限制在边界处，Core 不受第三方精度选择影响。
