# 8.10 常见应用案例

> **难度**：⭐⭐⭐⭐☆

## 核心内容

```
常见应用案例：

1. FPS 角色第三人称命中反应（最流行方案）
   spine_01 以上 PhysicalAnimation Profile "HitReact"
   命中时：AddImpulse + SetStrengthMultiplier(0.2f)
   0.5s 后：恢复 SetStrengthMultiplier(1.0f)

2. NPC 被推撞（无死亡时）
   角色被玩家撞击 → 身体自然晃动
   不需要完整布娃娃，Physical Animation 足够
   推撞力 < 阈值：PhysAnim 晃动
   推撞力 > 阈值：触发完整 Ragdoll

3. 载具乘客身体晃动
   车辆颠簸 → 乘客身体惯性延迟
   Physical Animation 弱追随（Strength=0.1~0.3）
   → 乘客看起来受到颠簸影响

4. 机器人/装甲人物的沉重感
   强 Physical Animation（Strength=2.0 超强追随）
   高 AngularVelocityStrength（高阻尼，减少抖动）
   → 骨骼紧密追随动画，但有明显惯性延迟

5. 布娃娃过渡中的引导（防止乱飞）
   完整 Ragdoll 时保留弱 PhysAnim（Strength=0.05）
   → 布娃娃不会完全无控，保持大致姿势
```
