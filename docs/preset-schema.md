# Preset Schema v1 契约

序列化器计划在 T021 中实现。本文在实施前确定兼容性格式。

```json
{
  "schemaVersion": 1,
  "appVersion": "0.1.0",
  "name": "Modern Lead",
  "inputGainDb": 0.0,
  "outputGainDb": -3.0,
  "nodes": [
    {
      "id": "000000000000001A",
      "type": "openrig.drive.soft_clip",
      "bypassed": false,
      "parameters": {
        "drive": 0.42,
        "level": 0.72
      }
    }
  ],
  "connections": [],
  "resources": []
}
```

## 规则

- `schemaVersion` 是迁移依据的整数；`appVersion` 用于说明和诊断。
- `NodeId` 使用 16 位十六进制字符串，保证全部 64 bit 在 JavaScript/Web 环境中可精确保存。
- Node `type` 和 Parameter key 是稳定的公开标识。
- 即使 v0.1 只提供线性链，也必须持久化 Connection。
- 在可行范围内保留未知的顶层字段与 Node 字段。
- 未知 Node type 应加载为自动 Bypass 的 `MissingNode`，并保留原始 JSON。
- 外部资源缺失不能导致整个 Preset 加载失败；所属 Node 应进入可见的缺失资源状态。

## ResourceRef

```json
{
  "kind": "nam",
  "path": "models/5150.nam",
  "sha256": "optional-lowercase-hex"
}
```

查找顺序：相对 Preset 的路径、OpenRig 资源库、已记录的绝对路径（若有）。保存时应优先使用可移植的相对路径或资源库引用；默认不把模型或 IR 字节嵌入 Preset。

## 迁移策略

- Parser 在构造 Graph 前验证结构与大小限制。
- 每次 Schema 变更都必须增加基于 Fixture 的迁移测试和 Round-trip 测试。
- 读取受支持的旧版本后，转换为当前内存模型；除非用户显式要求兼容导出，保存时统一写当前版本。
- 遇到更新但未知的 `schemaVersion`，应在非实时线程给出明确错误，不能部分激活音频状态。
