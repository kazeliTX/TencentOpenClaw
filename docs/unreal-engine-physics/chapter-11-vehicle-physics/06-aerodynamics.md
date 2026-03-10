# 11.6 空气动力学

> **难度**：⭐⭐⭐⭐⭐

```
Chaos Vehicle 空气动力学参数：

DragCoefficient（空气阻力系数）：
  F_drag = 0.5 × ρ × Cd × A × v²
  ρ = 空气密度（1.225 kg/m³）
  Cd 典型值：轿车 0.25~0.35，赛车 0.25，SUV 0.4
  
  在 UE 中：DragCoefficient 是综合系数
  增大 → 高速极速降低（但低速不受影响）
  
DownforceCoefficient（下压力系数）：
  高速时产生向下的力 → 轮胎抓地力增大
  赛车重要参数：高速弯道的稳定性
  
  F_downforce = 0.5 × ρ × Cl × A × v²
  Cl 典型值：普通车 0（无下压），F1 >3.0（极强下压）

UE 中手动实现下压力（ChaosVehicle 原生支持有限）：
  // Tick 中根据速度施加额外向下力
  float Speed = GetVelocity().Size();
  float Downforce = 0.001f * Speed * Speed * DownforceCoeff;
  VehicleMesh->AddForce(FVector(0, 0, -Downforce));
```
