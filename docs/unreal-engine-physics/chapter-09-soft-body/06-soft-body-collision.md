# 9.6 软体碰撞处理

> **难度**：⭐⭐⭐⭐☆

```
UCableComponent 碰撞限制：
  UCableComponent 本身没有碰撞 Body
  无法用 Trace 命中
  
  变通方案：
    在绳索关键节点位置生成不可见的小球碰撞体（SphereCollision）
    随绳索运动（Tick 更新位置）
    → 玩家可以"命中"绳索
    
物理绳索碰撞（Constraint Chain）：
  每节绳段有真实 Physics Body
  天然支持碰撞检测
  注意：相邻绳段应设置为忽略彼此碰撞
       否则相邻段会互相碰撞产生抖动

Chaos Cloth 作为软体碰撞：
  将软包用 Chaos Cloth 实现
  Cloth 有内置碰撞（与 Physics Asset Body 碰撞）
  → 可以被刚体压缩/推挤（近似软体交互）
```
