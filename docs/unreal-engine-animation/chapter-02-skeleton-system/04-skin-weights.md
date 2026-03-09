# 2.4 蒙皮权重原理与优化

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、蒙皮权重的定义

蒙皮权重（Skin Weights）决定了每个顶点受哪些骨骼影响以及影响程度：

```
顶点 V 的蒙皮权重示例（手腕区域）：

骨骼         权重
─────────────────
lowerarm_l   0.6    （主导：前臂骨骼）
hand_l       0.35   （次要：手部骨骼）
lowerarm_r   0.05   （极少量影响）
─────────────────
合计         1.0    ← 权重之和必须 = 1.0（归一化）
```

---

## 二、权重绘制原则

### 2.1 关节区域权重分布

```
关节截面权重分布（以膝关节为例）：

大腿骨权重：  1.0 ─────█████── 0.5 ──── 0.0
                    ↑大腿区    ↑膝关节   ↑小腿区
膝关节权重：  0.0 ──────── 0.0~0.5──── 0.0
小腿骨权重：  0.0 ──── 0.0 ──── 0.5 ─────████ 1.0

最佳实践：
  • 关节中心区域使用渐变过渡，避免硬边
  • 过渡区域宽度 ≈ 关节直径的 1/3 到 1/2
  • 避免单顶点权重超过 4 根骨骼（移动端尤其注意）
```

### 2.2 常见权重问题

| 问题 | 表现 | 原因 | 解决方案 |
|------|------|------|--------|
| 权重渗漏 | 远处部件随近处骨骼移动 | 权重归一化时波及远处顶点 | 检查并修正问题权重 |
| 关节凹陷 | 弯曲时关节处凹进去 | 权重过渡太硬，缺少辅助骨骼 | 添加辅助骨骼+平滑权重 |
| 体积丢失 | 肌肉弯曲时变薄 | LBS 算法固有问题 | 切换 DQS 或添加修正混合形 |
| 肩部穿模 | 手臂抬起时肩部穿入躯干 | 肩部权重处理不当 | 精细绘制锁骨/肩胛骨权重 |

---

## 三、在 UE 中查看和编辑蒙皮权重

### 3.1 权重可视化

```
操作步骤：
  1. 打开 Skeletal Mesh 编辑器（双击 SK_xxx 资产）
  2. 工具栏 → Skin Weights（权重可视化模式）
  3. 左侧骨骼树：点击任意骨骼 → 高亮该骨骼的权重分布
  4. 颜色含义：
     红色（亮）= 权重 1.0（完全由此骨骼控制）
     蓝色（暗）= 权重 0.0（不受此骨骼影响）
     渐变     = 过渡区域
```

### 3.2 运行时读取权重（C++）

```cpp
// 读取顶点蒙皮权重（需要 CPU Skinning 或通过 StaticMesh 方式）
void ReadSkinWeights(USkeletalMeshComponent* Mesh)
{
    if (!Mesh || !Mesh->GetSkeletalMeshAsset()) return;

    USkeletalMesh* SkelMesh = Mesh->GetSkeletalMeshAsset();

    // 获取 LOD0 的渲染数据
    FSkeletalMeshRenderData* RenderData = SkelMesh->GetResourceForRendering();
    if (!RenderData || RenderData->LODRenderData.Num() == 0) return;

    const FSkeletalMeshLODRenderData& LODData = RenderData->LODRenderData[0];

    // 遍历所有 Section
    for (int32 SectionIdx = 0; SectionIdx < LODData.RenderSections.Num(); ++SectionIdx)
    {
        const FSkelMeshRenderSection& Section = LODData.RenderSections[SectionIdx];

        UE_LOG(LogTemp, Log,
            TEXT("Section[%d]: BaseVertex=%d, NumVertices=%d, MaxBoneInfluences=%d"),
            SectionIdx,
            Section.BaseVertexIndex,
            Section.NumVertices,
            Section.MaxBoneInfluences);

        // 注意：直接读取顶点权重需要访问底层缓冲区
        // 通常通过编辑器工具或 FSkeletalMeshImportData 更方便
    }
}

// 获取某根骨骼影响的顶点数量（编辑器工具类）
// 注意：运行时权重读取需要开启 bAllowCPUAccess
int32 GetBoneInfluenceVertexCount(USkeletalMesh* SkelMesh, int32 BoneIdx)
{
    // 需要 SkelMesh->bAllowCPUAccess = true
    // 或在 LOD 设置中开启 "Allow CPU Access"
    // 实际项目中通常只在编辑器/工具代码中使用
    return -1; // 占位，实际实现依赖编辑器模块
}
```

---

## 四、权重优化建议

### 4.1 移动端权重优化

```
移动端（iOS / Android）硬件限制：
  • GPU Uniform Buffer 较小，骨骼数量上限低
  • 建议每个顶点最多 4 骨骼影响

优化步骤：
  1. 在 DCC 软件中：Skinning → Normalize Weights → Prune Small Weights (< 0.01)
  2. UE 导入设置：Max Bone Influences Per Vertex = 4
  3. LOD 设置：高 LOD（远处）进一步降低到 2 骨骼

代码配置：
  USkeletalMesh* Mesh = ...;
  Mesh->bUseBoundsOnlyForTemporalAA = false; // 可选优化
  // LOD 设置通过编辑器配置，不建议运行时修改
```

### 4.2 权重压缩

```
UE 5.3+ 支持蒙皮权重压缩：

项目设置 → Engine → Rendering → Skinning:
  Bone Influence Count Reduction:
    • Threshold Vertex Count Factor: 1.0
    • LOD Threshold: 配置哪个 LOD 开始降低骨骼数量

优化效果参考：
  从 8 骨骼影响 → 4 骨骼影响：内存减少约 33%
  视觉差距：通常不可见（特别是 LOD1 以上）
```

---

## 五、辅助骨骼（Corrective Bones）

辅助骨骼是解决蒙皮缺陷的传统方案：

```
示例：肩部辅助骨骼

问题：手臂抬高时，肩膀区域体积丢失

解决：添加肩部辅助骨骼（例如 "clavicle_helper_l"）
  • 辅助骨骼绑定在锁骨骨骼上
  • 通过 AnimGraph 中的 "Bone Driven Controller" 节点，
    让辅助骨骼根据上臂抬起角度自动旋转
  • 肩部顶点同时受锁骨和辅助骨骼权重影响
  • 效果：手臂抬起时，辅助骨骼补充肩部体积

UE5 中的替代方案：
  ML Deformer（机器学习变形器）— 比辅助骨骼更精确，但需要训练数据
```

---

## 六、延伸阅读

- 📄 [2.3 蒙皮算法：LBS vs DQS](./03-skinning-algorithms.md)
- 📄 [2.5 虚拟骨骼与辅助骨骼](./05-virtual-bones.md)
- 🔗 [UE Skinning Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/skeletal-mesh-rendering-paths-in-unreal-engine)
