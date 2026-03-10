# 5.2 Body（物理体）设置详解

> **难度**：⭐⭐⭐⭐☆

## 一、Body 形状类型

```
PhAT Body 支持三种形状：
  Sphere（球体）：头部、关节末端，性能最好
  Capsule（胶囊）：四肢躯干，最常用
  Box（箱体）：脚掌、手掌等扁平骨骼

尺寸调整技巧：
  在 PhAT Viewport 中直接拖拽控制柄
  或在 Details → Primitive → Radius/HalfHeight 输入精确数值
  
  开启 Simulation 模式后确认 Body 没有穿插
  相邻 Body 应轻微重叠（3-5cm）保证约束稳定
```

## 二、Body 物理属性

```
关键属性（Details → Physics）：
  Mass（质量）：
    MassInKg：手动指定质量（0=自动计算）
    自动计算公式：质量 = 密度 × 体积
    默认密度：1.0 g/cm³（= 水密度，合理近似人体）
    
  Linear/Angular Damping：
    局部阻尼，叠加全局设置
    布娃娃推荐：Linear=0.5, Angular=1.0
    水下布娃娃：Linear=5.0, Angular=5.0
    
  Max Angular Velocity：
    单个 Body 的最大角速度限制
    避免某些 Body 高速旋转导致爆炸

碰撞响应（Details → Collision）：
  Collision Response：通常 Ragdoll Profile
    Block World/WorldStatic
    Ignore Pawn（布娃娃之间不相互碰撞）
    Ignore Camera
```

## 三、PhysicalMaterial 绑定

```
每个 Body 可以绑定不同的 PhysicalMaterial：
  Details → Physics → Physical Material Override
  
应用场景：
  头部 Body → 玻璃材质（High Restitution）→ 头部弹弹的
  躯干 Body → 标准肉体材质（摩擦=0.8, 弹性=0.1）
  手部 Body → 手套材质（摩擦稍低）

布娃娃脚部防滑（特殊需求）：
  脚部 Body → 高摩擦材质
  → 布娃娃不会在斜坡上无限滑动
```

## 四、Body 质量分布

```
人形角色质量分布参考（70kg 总重）：
  骨骼              占比   质量(kg)
  pelvis+spine      35%    ~24
  thigh × 2         20%    ~14（各7）
  upperarm × 2       8%    ~6（各3）
  calf × 2           8%    ~6（各3）
  head+neck          8%    ~5.6
  foot × 2           5%    ~3.5
  lowerarm × 2       6%    ~4.2
  hand × 2           3%    ~2.1
  其余               7%    ~4.6

调整质量的作用：
  质心越低 → 布娃娃越稳定，不容易翻转
  四肢过重 → 布娃娃飞出时四肢摆动夸张
  头部过重 → 布娃娃容易头朝下倒
  
UE 中验证质量：
  开启 p.Chaos.DebugDraw.ShowMassProperties 1
```

## 五、延伸阅读

- 📄 [5.3 Constraint 配置](./03-constraint-setup.md)
