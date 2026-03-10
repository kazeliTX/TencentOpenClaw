# 8.6 与 AnimBP 的混合

> **难度**：⭐⭐⭐⭐☆

## 核心内容

```
Physical Animation 与 AnimBP 的配合：
  PhysicalAnimation 不直接修改 AnimBP 输出
  而是在物理层面施加弹簧力追随 AnimBP 的骨骼目标位置

AnimBP 中的配合：
  EventGraph：受击时设置变量 bIsHit = true
  AnimGraph：Layered Blend Per Bone → 受击时调整骨骼权重
  同时：PhysAnim->SetStrengthMultiplier(0.2f) 降低追随强度

最佳实践：
  AnimBP 处理宏观姿势（走路/跑步/蹲伏）
  PhysicalAnimation 处理微观物理（命中抖动/碰撞响应）
  两者不冲突，互补增强
```
