# 7.5 破碎阈值与连接强度

> **难度**：⭐⭐⭐⭐☆

## 一、Damage Threshold

```
Damage Threshold 数组：
  GC Details → Chaos Physics → Damage Threshold
  每个元素对应层级深度：[Level0, Level1, Level2, ...]
  
  值含义：破碎该层级所需的最小应变/冲击力
  
  材质参考值（相对值，实际需根据 Mass 调整）：
  
  材质           Level0   Level1   Level2
  薄玻璃          100      50       10
  厚玻璃          500      200      50
  陶瓷罐          300      100      20
  砖块（单砖）    800      300       -
  混凝土墙       3000     1000     200
  木板            400      150       -
  金属薄板        800      300      80
  石头           2000      800     200
  冰块            200       80      15
  
注意：
  值越大 = 越难破碎（需要更大的冲击/爆炸）
  与 GC 的 Mass 相关：质量越大，惯性越大，同样力下应变越小
  建议先用默认值，再在 PIE 中测试调整
```

## 二、连接强度 vs 破碎阈值

```
两个控制破碎的参数：

1. Damage Threshold（伤害阈值）
   外部施加的力/应变 > 此值 → 碎片连接断开
   全局参数（影响所有碎片）

2. Internal Strain（内部应变强度）
   每条碎片间连接边的强度
   可以在 Fracture 工具中逐连接绘制
   
   高 Internal Strain 连接 = 牢固连接（需要更大冲击力才能断开）
   低 Internal Strain 连接 = 脆弱连接（轻易断开）

组合使用：
  整体 Damage Threshold 设高（不容易破碎）
  关键区域（窗户/裂缝）绘制低 Internal Strain
  → 只有特定区域容易破碎（窗户先碎，墙体后碎）
```

## 三、累积伤害模型

```
Damage Model = Accumulated（累积模式）：
  每次碰撞累积一定伤害
  累积总伤害 > Damage Threshold → 破碎
  
  适合：
    多次子弹射击才能打碎的混凝土
    重复打击才会破裂的木头
    
  参数：
    Damage Reset Interval：
      累积伤害的重置周期（秒）
      = 0.0 → 永久累积（只要打够就会碎）
      > 0.0 → N 秒后伤害重置（强硬回复）
    
    Minimum Mass Threshold：
      质量低于此值的碎片自动停止物理（节省性能）
```

## 四、延伸阅读

- 📄 [7.6 Anchor Fields](./06-anchor-fields.md)
- 📄 [7.7 破碎事件](./07-fracture-events.md)
