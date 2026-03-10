# 10.2 Niagara Fluids 配置

> **难度**：⭐⭐⭐⭐⭐

## 一、Niagara Fluids 概述

```
Niagara Fluids（UE5+）：
  基于 GPU 的 Eulerian 流体（格网方法）
  支持：2D 浅水 / 3D 烟雾 / 3D 液体
  
  激活方法：
    Edit → Plugins → Niagara Fluids ✓
    重启编辑器

  内置模板：
    Grid2D_Shallow_Water：2D 浅水（水面波纹最常用）
    Grid3D_Gas：3D 气体/烟雾
    Grid3D_Liquid：3D 液体（实验性，开销巨大）

创建步骤（2D 浅水波纹）：
  1. 内容浏览器 → 新建 → Niagara System
  2. 选择模板：Grid2D_Shallow_Water
  3. 参数调整：
     Grid Size：模拟区域大小（cm），通常 200~500
     Grid Resolution：分辨率（32~128，越高越细腻越慢）
     Viscosity：粘度（0=水，1=蜂蜜）
     Surface Tension：表面张力
  4. 在场景中放置 Niagara Component
  5. 绑定到水面材质（RenderTarget 输出）
```

## 二、添加外力（角色踩水）

```
通过 Niagara 参数接口在运行时添加外力：

// C++ 中向 Niagara Fluid 施加力（产生水波）
UFUNCTION(BlueprintCallable)
void SplashAtLocation(UNiagaraComponent* FluidComp,
    FVector WorldPos, float Strength)
{
    // 获取局部坐标（Niagara Grid 用归一化坐标）
    FVector LocalPos = FluidComp->GetComponentTransform()
        .InverseTransformPosition(WorldPos);
    
    // 设置 User Parameter（在 Niagara 系统中暴露的参数）
    FluidComp->SetNiagaraVariableVec3(
        TEXT("User.SplashPosition"), LocalPos);
    FluidComp->SetNiagaraVariableFloat(
        TEXT("User.SplashStrength"), Strength);
    FluidComp->SetNiagaraVariableBool(
        TEXT("User.bSplash"), true);
    
    // 下帧重置
    FTimerHandle T;
    GetWorld()->GetTimerManager().SetTimer(T,
        FTimerDelegate::CreateWeakLambda(this, [FluidComp]() {
            if (FluidComp)
                FluidComp->SetNiagaraVariableBool(
                    TEXT("User.bSplash"), false);
        }), 0.05f, false);
}
```

## 三、延伸阅读

- 📄 [10.3 Water Plugin](./03-water-plugin.md)
