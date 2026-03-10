# 7.10 破碎常见实现模式

> **难度**：⭐⭐⭐⭐☆

## 一、可修复的破碎物

```cpp
// 破碎后可以重置（关卡重置/复活）
UCLASS()
class AResettableDestructible : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UGeometryCollectionComponent* GCC;
    bool bBroken = false;

    UFUNCTION(BlueprintCallable)
    void ResetToIntact()
    {
        // 重新创建 GCC（销毁旧的，生成新的）
        GCC->DestroyComponent();
        GCC = NewObject<UGeometryCollectionComponent>(this);
        GCC->SetGeometryCollection(OriginalGCAsset);
        GCC->RegisterComponent();
        GCC->AttachToComponent(RootComponent,
            FAttachmentTransformRules::SnapToTargetIncludingScale);
        bBroken = false;
    }
    
    UPROPERTY(EditAnywhere) UGeometryCollection* OriginalGCAsset;
};
```

## 二、子弹穿孔效果

```
模拟子弹打墙留下的洞（不完全破碎）：
  
方案 A：局部 Field Strain（推荐）
  子弹命中点 → 在小半径（5-15cm）内施加高 Strain
  只破碎命中点周围的 Level 0 碎片
  → 局部小坑效果，墙体大部分完整

方案 B：Decal + 弱 Fracture
  近处：真实破碎（小半径 GC Field）
  远处：只播放弹孔 Decal（不触发物理破碎）
  距离阈值：300cm
  → 玩家近看是真实破碎，远看是贴花，性能平衡

方案 C：预制弹孔 Mesh
  在弹孔位置生成预制的"已破碎"StaticMesh
  同时将该区域的 GC 碎片设为 Hidden
  最低性能开销，效果较差
```

## 三、建筑倒塌时序控制

```
有节奏的倒塌（电影感）：

1. 底层爆破：
   施加应变 → 底层碎片开始下落
   
2. 0.2s 后：中层开始崩塌（因为底层支撑消失）

3. 0.5s 后：顶层整体滑落

4. 1.0s 后：大量碎片落地，生成粉尘特效

实现：
  通过 Timer 序列 + 多个 AnchorField 逐步解锁
  每层解锁时施加小 Strain（辅助触发破碎）
  配合 CameraShake + 音效 → 极具冲击力
```

## 四、延伸阅读

- 📄 [代码示例：破碎触发](./code/01_fracture_trigger.cpp)
- 📄 [代码示例：破碎管理器](./code/03_destruction_manager.cpp)
