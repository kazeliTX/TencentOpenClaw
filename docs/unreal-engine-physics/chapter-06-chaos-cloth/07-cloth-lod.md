# 6.7 布料 LOD 与性能设置

> **难度**：⭐⭐⭐⭐☆

## 一、布料 LOD 策略

```
布料 LOD 与 SkeletalMesh LOD 绑定：

LOD 0（近距离，<300cm）：
  完整布料模拟
  自碰撞：开启
  迭代次数：8
  
LOD 1（中距离，300~1000cm）：
  简化布料模拟
  自碰撞：关闭（节省大量性能）
  迭代次数：4
  
LOD 2（远距离，>1000cm）：
  完全禁用布料模拟
  布料跟随骨骼动画（无物理）
  SkeletalMesh LOD 用低面数模型

配置方式：
  每个 SkeletalMesh LOD 可以分配不同的 ClothAsset
  或在同一 ClothAsset 中配置不同 LOD 的参数
  
  SkeletalMesh 编辑器 → LOD Settings → LOD Screen Size
  → 调整 LOD 切换距离
```

## 二、布料性能预算

```
开销估算（每个布料 Component，中端 PC）：

顶点数    自碰撞    迭代次数   每帧时间(ms)
500       Off       4         0.2
1000      Off       8         0.5
2000      Off       8         1.0
1000      On        8         2.5
2000      On        8         6.0    ← 开始昂贵
4000      On       12         20+    ← 不可接受

优化建议：
  单角色布料顶点总数 < 3000
  同屏布料 Component < 8 个
  超出视距的角色：DisableClothSimulation()
  
  // 批量关闭布料
  void SetClothEnabled(ACharacter* Char, bool bEnable)
  {
      USkeletalMeshComponent* Mesh = Char->GetMesh();
      if (bEnable)
          Mesh->ResumeClothingSimulation();
      else
          Mesh->SuspendClothingSimulation();
  }
```

## 三、Chaos Cloth 全局优化设置

```
Project Settings → Engine → Physics → Chaos:
  Cloth Max Num Iterations：全局迭代次数上限
  Cloth Solver Frequency  ：求解器频率（默认=物理帧率）

控制台命令（调试用）：
  p.ChaosCloth.Enable 1/0              ← 全局开/关布料
  p.ChaosCloth.MaxNumIterations 4      ← 强制限制迭代次数
  p.ChaosCloth.NumSubsteps 1           ← 子步数（减少节省性能）
  p.ChaosCloth.SleepThreshold 1.0      ← 布料睡眠阈值
  
Cloth LOD 偏移（手动降档）：
  Mesh->SetClothingSimulationQuality(EClothingSimulationQuality::Low);
  // Low / Medium / High 三档
```

## 四、延伸阅读

- 📄 [6.8 Chaos Cloth 编辑器详解](./08-chaos-cloth-panel.md)
