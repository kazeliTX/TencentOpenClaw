# 9.10 应用案例（帐篷/网/旗帜）

> **难度**：⭐⭐⭐⭐☆

```
帐篷（固定点布料）：
  使用 Chaos Cloth
  4 个角顶点 MaxDistance = 0（固定住）
  中间区域 MaxDistance = 30~50（可随风摆动）
  大风时帐篷边缘振动，内部被撑开

渔网：
  Chaos Cloth，网格 Mesh 做成网眼形状
  Stretch Stiffness 极低（0.1）→ 网眼可拉伸
  与鱼/物体碰撞 → 网形变
  
旗帜（参考 Ch06 参数表）：
  丝绸参数，顶端骨骼固定
  风力驱动飘动

吊桥桥板（物理约束链）：
  每块桥板 = 一个有 Physics Body 的 SM
  相邻桥板用 PhysicsConstraint 连接
  两端锚定在桥柱上（Kinematic）
  玩家走上 → 桥板下沉
  绳索断裂 → 桥板飞散

蜘蛛网：
  UCableComponent（纯视觉，性能好）
  多根 Cable 从网中心向外辐射 + 环形连接
  命中时播放破碎动画（不需要真实物理撕裂）
```
