# 3.10 绳索与链条（Cable Component / 物理绳）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 35 分钟

---

## 一、UCableComponent（视觉绳索）

```
UCableComponent：
  内置插件：Cable Component（默认已启用）
  用途：视觉效果绳索，不参与碰撞/物理
  实现：XPBD 约束链（不基于 Chaos 求解器）

特点：
  高性能（不进入 Chaos 物理流水线）
  支持挂载端点到骨骼/Component
  可设置重力缩放、风场影响
  只有视觉（子弹打不断，不能挂载重物的物理反应）

参数：
  Cable Length：绳索总长度（cm）
  Num Segments：绳索分段数（越多越柔软，越耗性能）
    推荐：10~20 段（远景），40~60 段（近景特写）
  Gravity Scale：重力对绳索的影响（默认1.0）
  Solve Iterations：XPBD 迭代次数（默认1，越高越精确）
  Enable Stiffness：启用刚度（绳索不那么软）
  Attach Start/End：端点是否附着
  
  Attach To 端点：
    Cable.SetAttachEndToComponent(OtherComp, SocketName);

用途：
  桥梁悬索（视觉）
  角色身上的挂饰（锁链、腰带）
  灯笼吊线
  电线/电缆（场景装饰）
```

---

## 二、物理绳索（弹簧链约束）

```
真正参与物理的绳索 = 多段刚体 + 约束链

架构：
  Segment[0] → Segment[1] → ... → Segment[N]
  每段：小型 Static Mesh（胶囊/盒）+ 物理模拟
  每对相邻段：PhysicsConstraint（球窝 + 弹性）

优点：
  真实物理：子弹可以切断，重物可以拉扯
  角色可以攀爬

缺点：
  性能高（N 个物理刚体 + N 个约束）
  稳定性需要调优（子步进，阻尼）

物理绳索约束配置（每节）：
  Linear X/Y/Z：Limited (Limit=2cm，允许微小弹性)
  Angular Swing 1/2：Limited (Limit=45°，弯曲限制)
  Angular Twist：Free
  
  Soft Linear Limit：true，Stiffness=1000，Damping=50
  Soft Swing Limit：true，Stiffness=200，Damping=30
  
  → 绳索每节可以微弯，但不能完全折断（受 Limit 约束）

断裂设置（可断绳索）：
  Linear Breakable Force：超过此力断开（绳子切断效果）
  Angular Breakable Torque：超过此力矩断开（弯折断裂）
  OnConstraintBroken 事件 → 断裂时触发（播放音效/粒子）
```

---

## 三、CableComponent 与物理绳索对比

```
──────────────────────────────────────────────────────────────
特性                UCableComponent    物理绳索（约束链）
──────────────────────────────────────────────────────────────
性能                高（O(N)迭代）      低（N个物理体+N约束）
视觉质量             高                  中（分段明显）
物理交互（碰撞）      否                  是
可被切断             否                  是（Breakable）
挂载重物物理反应      否                  是
风场/粒子交互         有限（Cable参数）    是（被动力影响）
调试难度             低                  高
──────────────────────────────────────────────────────────────

推荐场景：
  视觉装饰绳索 → UCableComponent（场景电线/悬索/饰品）
  可交互绳索（攀爬/切断/挂重物）→ 物理约束链
  游戏玩法绳钩（Grappling Hook）→ 射线 + 角色约束（不需要绳子物理）
```

---

## 四、延伸阅读

- 📄 [代码示例：物理绳索](./code/03_rope_chain.cpp)
- 🔗 [Cable Component Plugin](https://dev.epicgames.com/documentation/en-us/unreal-engine/cable-component-plugin-in-unreal-engine)
- 🔗 [Physics Constraints](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-constraints-in-unreal-engine)
