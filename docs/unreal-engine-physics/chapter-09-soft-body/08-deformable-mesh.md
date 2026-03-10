# 9.8 可变形网格体

> **难度**：⭐⭐⭐⭐⭐

```
UE5 可变形 Mesh 方案：

1. Chaos Deformable（实验性）
   配合 Chaos Soft Body 使用
   Mesh 顶点跟随软体粒子运动
   → 真正的网格变形（运行时顶点移位）
   开销极大，生产环境暂不推荐

2. GeometryScript（运行时 Mesh 修改）
   UE5 新增，支持 C++ 和蓝图修改网格顶点
   可以在命中点凹陷、拉伸、扭曲
   配合 DynamicMeshComponent 使用
   
   // 示例：命中点局部凹陷
   UDynamicMesh* DM = DynMeshComp->GetDynamicMesh();
   UGeometryScriptLibrary_MeshDeformFunctions::ApplyLinearDisplacement(
       DM, NearestPoint, -HitNormal * 5.f, ...);

3. RenderTarget 驱动的顶点位移（性能最佳）
   命中点写入 RenderTarget
   材质的 World Position Offset 读取 RenderTarget
   → GPU 端顶点位移（近乎零 CPU 开销）
   → 适合草地/沙地/雪地脚印
```
