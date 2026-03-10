# 10.10 应用案例

> **难度**：⭐⭐⭐⭐☆

```
典型流体应用案例：

1. 开放世界海洋（大型 RPG/动作游戏）
   Water Plugin WaterBody_Ocean
   Gerstner Wave 8层叠加
   BuoyancyComponent 实现船只/浮桶漂浮
   岸边泡沫：FoamDepthThreshold = 200cm
   
2. 角色进水涟漪（FPS/TPS）
   Niagara Grid2D_Shallow_Water（64×64）
   放置在角色脚下（随角色移动）
   脚步事件 → SplashAtLocation → 涟漪
   配合水花粒子 + 溅水音效
   
3. 河流急流关卡
   WaterBody_River + 样条线路径
   角色进入河流 → 受到 Current Force（沿河流方向施加力）
   WaterBodyComponent→GetWaterCurrentAtLocation → 获取水流向量
   
4. 下雨水坑积水
   Niagara Fluids 小水坑（32×32，极低开销）
   雨滴粒子命中 → 触发涟漪
   水坑材质随时间加深（渐进积水效果）
   
5. 水下视觉效果
   角色头部进入水面以下 → PostProcess：
     SceneColorTint（蓝绿色）
     DepthOfField（水下模糊）
     Niagara 气泡粒子
   通过水面高度 vs 摄像机 Z 轴判断是否在水下
```
