# 8.8 瞄准辅助物理效果

> **难度**：⭐⭐⭐⭐☆

## 核心内容

```
瞄准时的物理效果：
  角色瞄准 → 上半身转向目标（AimOffset 节点控制宏观方向）
  同时身体自然有细微晃动（呼吸/手抖）

用 Physical Animation 实现：
  1. AimOffset AnimBP 节点控制宏观方向
  2. Physical Animation 弱追随（Strength = 0.3）
  → 瞄准方向准确，但有轻微物理晃动（逼真感）

手臂的武器晃动：
  武器 Mesh 附加在 hand_r 上
  hand_r 有 PhysicalAnimation 弱物理（OrientationStrength=200）
  → 武器在瞄准时有微小抖动（符合真实感）

屏住呼吸效果（狙击）：
  按住 ADS 键 → SetStrengthMultiplier(3.0f) → 追随更紧，晃动减少
  配合呼吸曲线 AnimBP 节点 → 视觉上身体慢慢稳定
```
