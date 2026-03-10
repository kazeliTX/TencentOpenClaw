# 9.7 果冻/软包模拟

> **难度**：⭐⭐⭐⭐⭐

```
果冻感实现方案（按推荐度排序）：

方案A：Chaos Cloth（推荐，稳定）
  将软包 Mesh 作为 Cloth 处理
  Stretch Stiffness = 0.95（几乎不拉伸，保持体积）
  Bend Stiffness = 0.5（可弯曲）
  UniformMass = 0.1 kg（轻）
  Damping = 0.1（快速稳定）
  → 视觉上类似果冻，但无法被压缩（Cloth 不处理体积保持）

方案B：Chaos Soft Body（实验性）
  真正的体积保持软体
  VolumeConservation = 0.9（高体积保持）
  Strain = 0.2（低刚度，果冻感）

方案C：顶点动画（最轻量）
  预制果冻晃动的顶点动画序列（离线烘焙）
  运行时播放顶点动画
  接受命中时切换不同动画
  → 无物理开销，效果固定但视觉良好
  → 适合大量 NPC（如多个果冻怪）
```
