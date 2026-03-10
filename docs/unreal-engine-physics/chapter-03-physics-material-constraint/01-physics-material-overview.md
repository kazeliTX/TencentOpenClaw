# 3.1 物理材质总览与创建

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 30 分钟

---

## 一、物理材质（Physical Material）

```
物理材质 ≠ 渲染材质（Material）！
  渲染材质：控制外观（颜色/纹理/法线/PBR参数）
  物理材质：控制物理行为（摩擦/弹性/密度/阻尼）

物理材质的作用：
  1. 决定两个表面接触时的摩擦力大小
  2. 决定碰撞后的弹跳强度（Restitution）
  3. 为碰撞体提供密度（用于自动质量计算）
  4. 标识表面类型（SurfaceType），用于音效/粒子判断
  5. 提供额外阻尼（可覆盖 Body 的默认阻尼）
```

---

## 二、创建物理材质

```
方法一（内容浏览器）：
  右键 → Physics → Physical Material
  命名（如 PM_Stone, PM_Ice, PM_Wood）

方法二（C++ 代码创建）：
  UPhysicalMaterial* PM = NewObject<UPhysicalMaterial>(this);
  PM->Friction          = 0.3f;
  PM->Restitution       = 0.4f;
  PM->Density           = 2.5f;
  PM->SurfaceType       = SurfaceType1;  // 对应 EPhysicalSurface 枚举

赋值方式：
  A. 在 Static Mesh 的 Material 槽 → Physical Material 字段
  B. Static Mesh → Details → Physics → Phys Material Override
  C. 运行时 C++ 动态赋值（见代码示例）

优先级（由低到高）：
  1. 项目默认物理材质（Project Settings → Physics → Default PhysMaterial）
  2. Mesh 的 Material 自带的 PhysMaterial
  3. Component 的 Phys Material Override（最高优先级，覆盖前两者）
```

---

## 三、物理材质全参数

```
─────────────────────────────────────────────────────────────
参数                  范围        默认   说明
─────────────────────────────────────────────────────────────
Friction              0.0~无穷    0.7    静摩擦系数
Static Friction       0.0~无穷    0.7    静摩擦（开始滑动的阻力）
Restitution           0.0~1.0    0.3    弹性恢复系数（0=不弹，1=完美弹）
Density               0.01~无穷  1.0    密度（g/cm³），影响自动质量计算
RaiseMassToPower      0.0~1.0    0.75   大体积物体质量缩放指数（防止巨物过重）
─────────────────────────────────────────────────────────────
SurfaceType           枚举        None   表面类型（触发音效/粒子判断）
─────────────────────────────────────────────────────────────
Override Restitution  bool        false  覆盖默认弹性
Override Friction     bool        false  覆盖默认摩擦
─────────────────────────────────────────────────────────────
Friction Combine Mode        Average/Min/Max/Multiply
Restitution Combine Mode     Average/Min/Max/Multiply
─────────────────────────────────────────────────────────────

Combine Mode 说明（两个接触表面的材质值如何组合）：
  Average:  (A + B) / 2     最常用，平均两者
  Min:      min(A, B)       取较小值（如冰面遇任何材质都滑）
  Max:      max(A, B)       取较大值（如橡胶遇任何材质都有高摩擦）
  Multiply: A * B           相乘（通常值<1时减小）

例：
  冰面 (Friction=0.02, CombineMode=Min)
  玩家靴子 (Friction=0.8)
  组合结果: min(0.02, 0.8) = 0.02  ← 冰面效果！
  → 无论玩家靴子摩擦多大，踩冰面都是 0.02
```

---

## 四、常见物理材质参考值

```
──────────────────────────────────────────────────────────────────
材质              Friction  Restitution  Density  SurfaceType
──────────────────────────────────────────────────────────────────
混凝土/石头       0.7       0.1          2.4      Stone
木材（干燥）      0.5       0.2          0.6      Wood
金属（钢）        0.5       0.3          7.8      Metal
冰面              0.02      0.1          0.9      Ice
橡胶              0.9       0.7          1.1      Rubber
泥土              0.4       0.05         1.6      Dirt
沙地              0.3       0.05         1.5      Sand
玻璃              0.4       0.5          2.5      Glass
肉体/布料         0.3       0.05         1.0      Flesh
水面（接触）      0.1       0.0          1.0      Water
草地              0.5       0.05         1.2      Grass
──────────────────────────────────────────────────────────────────
注：密度单位 g/cm³，UE 中以 1g/cm³ 为默认
```

---

## 五、物理材质与 Hit 事件结合

```cpp
// 在 Hit 回调中获取物理材质，决定音效类型
void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NI, const FHitResult& Hit)
{
    if (!Hit.PhysMaterial.IsValid()) return;
    
    EPhysicalSurface Surface = Hit.PhysMaterial->SurfaceType;
    
    USoundBase* ImpactSound = nullptr;
    switch (Surface)
    {
        case SurfaceType1:  // Stone
            ImpactSound = StoneImpactSound; break;
        case SurfaceType2:  // Wood
            ImpactSound = WoodImpactSound;  break;
        case SurfaceType3:  // Metal
            ImpactSound = MetalImpactSound; break;
        default:
            ImpactSound = DefaultImpactSound;
    }
    
    if (ImpactSound)
        UGameplayStatics::PlaySoundAtLocation(
            this, ImpactSound, Hit.ImpactPoint,
            FMath::GetMappedRangeValueClamped(
                FVector2D(0,200000), FVector2D(0.1f,1.f),
                NI.Size())); // 音量随冲量变化
}
```

---

## 六、延伸阅读

- 📄 [3.2 摩擦力与弹性详解](./02-friction-restitution.md)
- 📄 [3.3 表面类型与音效粒子](./03-surface-type.md)
- 🔗 [Physical Materials 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/physical-materials-in-unreal-engine)
