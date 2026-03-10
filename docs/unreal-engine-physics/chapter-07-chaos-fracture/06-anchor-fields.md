# 7.6 锚定场（Anchor Fields）

> **难度**：⭐⭐⭐⭐☆

## 一、锚定场的作用

```
锚定场（Anchor Field）控制 GC 碎片的物理状态：
  Static（静止）：碎片固定，不参与物理（类似 Kinematic）
  Dynamic（动态）：碎片激活，参与物理模拟
  Sleeping（睡眠）：碎片静止但可以被唤醒
  
  典型用途：
    建筑底部用 AnchorField = Static
    → 底部地基固定，只有被攻击的上部才会掉落
    
    玩家破门进入时：
    → 门锁区域 Static，被踢之后 Dynamic
```

## 二、AnchorField 配置

```
放置方式：
  Fracture Mode → Fields → Anchor Field Actor
  调整尺寸覆盖需要固定的区域

蓝图节点方式：
  Apply Anchor Field（Anchor Field Actor 上的蓝图调用）
  → 选择 State：Static / Dynamic / Sleeping

C++ 方式：
  // GeometryCollectionComponent 上直接设置初始状态
  GCComp->InitializationFields.Add(AnchorFieldActor);
  
  // 运行时动态改变（碰炸某区域时解锁）
  // 用 ExternalStrain Field 超过 Static 碎片的阈值 → 变为 Dynamic
```

## 三、分段倒塌实现

```
大型建筑倒塌效果（典型工程案例）：

阶段设计：
  初始状态：AnchorField 覆盖整个建筑 → 全部 Static
  
  第一次爆炸：移除底层 AnchorField → 底层 Dynamic 开始倒塌
  
  第二次爆炸：移除中层 AnchorField → 中层跟着倒
  
  最终崩塌：全部 Dynamic → 整体倒塌

对应 Field Sequence：
  FieldAnchor_Layer1->SetActorHiddenInGame(true);  // 底层解锁
  // 等待延迟
  FieldAnchor_Layer2->SetActorHiddenInGame(true);  // 中层解锁
  
  注意：隐藏 Actor ≠ 撤销 Field 影响
  正确做法：用 UFieldSystemComponent 动态添加/移除 Field
```

## 四、延伸阅读

- 📄 [7.7 破碎事件](./07-fracture-events.md)
- 📄 [7.8 程序化破碎](./08-procedural-fracture.md)
