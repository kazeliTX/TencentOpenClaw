# 7.6 Groom 毛发模拟

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 20 分钟

---

## 一、Groom 系统概述

```
UE5 Groom 系统提供真实的毛发/毛皮渲染和模拟：
  基于 Strand 的渲染（每根发丝单独渲染）
  集成 Niagara 物理模拟（发丝跟随角色运动飘动）

需要启用插件：Groom（Edit → Plugins → Rendering → Groom）

Groom Asset 来源：
  从 Maya/Houdini 导出 .usd / .abc 格式
  Maya XGen → 导出 Alembic → UE Groom 导入器
```

---

## 二、Groom 组件配置

```cpp
// 添加 Groom 组件到角色
UPROPERTY(VisibleAnywhere) UGroomComponent* HairComponent;

// 构造函数
AMyCharacter::AMyCharacter()
{
    HairComponent = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
    HairComponent->SetupAttachment(GetMesh(), FName("head")); // 附着到头骨骼
}
```

---

## 三、物理模拟配置

```
Groom Asset → Physics 面板：

Enable Simulation: ✅（启用物理模拟）

Solver Settings:
  Iterations:     4     （求解迭代次数，越高越稳定但越慢）
  Sub Steps:      1     （物理子步数）

Simulation Type: Follow Roots（发根跟随骨骼，发梢物理飘动）

External Forces:
  Gravity Scale:  0.1   （发丝受重力影响，通常比真实重力小）
  Air Drag:       0.05  （空气阻力）

Stiffness:
  Bend:  0.3            （弯曲刚度，越高发丝越硬）
  Stretch: 1.0          （拉伸刚度，通常保持 1.0 防拉伸）
  
Collision:
  Radius: 0.5           （发丝碰撞半径）
  Friction: 0.1         （与碰撞体的摩擦力）
```

---

## 四、Groom 绑定（Groom Binding）

```
Groom 需要与骨骼 Mesh 绑定才能跟随骨骼运动：

创建 Groom Binding：
  内容浏览器 → 右键 Groom Asset → Create Binding
  Skeletal Mesh: 你的角色 Mesh
  Source Skeletal Mesh: 导出 Groom 时的参考 Mesh（通常相同）

在 Groom 组件中设置绑定：
  Binding Asset: 刚创建的 Groom Binding
  → Groom 现在会精确跟随骨骼变形
```

---

## 五、LOD 与性能

```
Groom 性能开销较高：

不同 LOD 策略：
  LOD 0（<5m）：完整发丝渲染 + 物理模拟
  LOD 1（5~15m）：减少发丝数量（50%），降低模拟频率
  LOD 2（>15m）：换为 Cards（平面发片）或 Mesh 发型
  LOD 3（>30m）：完全使用 Mesh 发型（无模拟）

Cards 模式（LOD 替代）：
  用少量多边形面片模拟发型
  比发丝渲染快 5~10 倍
  Groom Asset → Generate Cards（自动生成）
```

---

## 六、延伸阅读

- 🔗 [Groom 系统文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/hair-rendering-and-simulation-in-unreal-engine)
- 🔗 [Groom LOD 指南](https://dev.epicgames.com/documentation/en-us/unreal-engine/groom-level-of-detail-in-unreal-engine)
