# 1.5 骨骼空间与坐标系

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟
> 这是很多人最容易混淆的概念，务必掌握！

---

## 一、UE 的世界坐标系

UE 使用**左手坐标系，Z-Up**（与 Maya 的 Y-Up 不同）：

```
UE 世界坐标系：

        Z（上）
        │
        │
        └────── Y（右）
       ╱
      X（前）

• X 轴：向前（Forward）
• Y 轴：向右（Right）
• Z 轴：向上（Up）
• 单位：厘米（cm），1 UU = 1 cm
```

---

## 二、三种骨骼空间

在 UE 动画系统中，骨骼变换存在三个不同的参考空间：

### 2.1 Local Space（局部空间）

```
定义：骨骼的变换相对于其【父骨骼】

示例：
  pelvis（盆骨）的 Local Transform = 相对于 root 的偏移
  spine_01 的 Local Transform = 相对于 pelvis 的偏移

特点：
• 动画数据默认存储格式（AnimSequence 中记录的就是 Local Transform）
• 修改某骨骼的 Local Transform 会影响其所有子骨骼
• 计算量最小，是动画系统内部的主要工作空间

访问方式（C++）：
FTransform LocalTransform = SkeletalMesh->GetBoneTransform(
    BoneIdx, 
    EBoneSpaces::LocalSpace
);
```

### 2.2 Component Space（组件空间）

```
定义：骨骼的变换相对于【SkeletalMeshComponent 的原点】

示例：
  骨骼 head 在角色体内的相对位置（如距离原点 170cm 高）

特点：
• IK 计算、Look At、骨骼控制节点都在此空间工作
• 需要从 Local Space 通过正向运动学链式计算得到
• 修改此空间变换后需要反向传播到 Local Space（较贵）

访问方式（C++）：
FTransform ComponentTransform = SkeletalMesh->GetBoneTransform(
    BoneIdx, 
    EBoneSpaces::ComponentSpace  // 默认值
);

// 或通过骨骼名称：
FTransform CompT = SkeletalMesh->GetBoneTransform(
    SkeletalMesh->GetBoneIndex(FName("head"))
);
```

### 2.3 World Space（世界空间）

```
定义：骨骼在【游戏世界】中的绝对位置

计算方式：
  BoneWorld = BoneComponent × ComponentWorld

特点：
• 最直观，但计算成本最高
• 用于物理碰撞、武器轨迹检测、特效生成等
• 不建议在 AnimGraph 内大量使用

访问方式（C++）：
// 方法1：通过 GetSocketTransform
FTransform WorldTransform = SkeletalMesh->GetSocketTransform(
    FName("head"),
    ERelativeTransformSpace::RTS_World
);

// 方法2：骨骼变换 × 组件世界变换
FTransform CompTransform = SkeletalMesh->GetBoneTransform(BoneIdx);
FTransform WorldT = CompTransform * SkeletalMesh->GetComponentTransform();
```

---

## 三、空间转换

### 3.1 Local → Component（正向运动学）

```
FK（Forward Kinematics）链式计算：

root_component = root_local（根骨骼无父级，两者相同）
pelvis_component = pelvis_local × root_component
spine_01_comp   = spine_01_local × pelvis_component
spine_02_comp   = spine_02_local × spine_01_comp
...
head_comp       = head_local × neck_comp

// UE 内部通过 FillComponentSpaceTransforms() 完成此计算
```

### 3.2 Component → Local（反向，用于 IK 输出）

```
IK 求解在 Component Space 中计算出目标变换后，需要转回 Local Space：

// 已知 head 的 Component Space 目标位置，求 head 的 Local Transform：
FTransform TargetComp = ...; // IK 计算结果
FTransform ParentComp = GetBoneTransform(neck_idx); // 父骨骼 Component 变换

FTransform NewLocal = TargetComp.GetRelativeTransform(ParentComp);
// 等价于：NewLocal = TargetComp × ParentComp.Inverse()
```

### 3.3 AnimGraph 中的空间转换节点

```
Convert Spaces 节点（在 AnimGraph 中）：
• Local to Component（用于进入 IK 节点前）
• Component to Local（用于 IK 节点输出后）

大多数 IK 和骨骼控制节点自动处理空间转换，
不需要手动添加 Convert Spaces 节点。
```

---

## 四、FTransform 深度解析

```cpp
// FTransform 是 UE 中表示变换的核心结构
// 内部存储：Translation(FVector) + Rotation(FQuat) + Scale(FVector)

FTransform T;

// 构造
T = FTransform(Rotation, Translation, Scale);
T = FTransform::Identity;  // 单位变换

// 访问
FVector    Loc   = T.GetLocation();
FQuat      Rot   = T.GetRotation();
FRotator   RotR  = T.GetRotation().Rotator();  // 转为欧拉角
FVector    Scale = T.GetScale3D();
FMatrix    Mat   = T.ToMatrixWithScale();       // 转为矩阵

// 修改
T.SetLocation(FVector(0, 0, 100));
T.SetRotation(FQuat(FRotator(0, 90, 0)));
T.SetScale3D(FVector(1, 1, 1));

// 变换组合（子 × 父 = 子的世界变换）
FTransform Child_World = Child_Local * Parent_World;

// 求相对变换（已知父和子世界变换，求子的局部变换）
FTransform Child_Local = Child_World.GetRelativeTransform(Parent_World);

// 变换一个点（局部点 → 世界点）
FVector WorldPoint = T.TransformPosition(LocalPoint);

// 逆变换一个点（世界点 → 局部点）
FVector LocalPoint = T.InverseTransformPosition(WorldPoint);

// 变换一个方向（不受位移影响）
FVector WorldDir = T.TransformVector(LocalDir);
```

---

## 五、FQuat（四元数）与旋转

动画系统内部使用四元数表示旋转，理解四元数对动画混合至关重要：

```cpp
// 四元数基础
FQuat Q = FQuat::Identity;          // 无旋转
FQuat Q2 = FQuat(FRotator(0,90,0)); // 从欧拉角构造
FQuat Q3 = FQuat(FVector(0,0,1), FMath::DegreesToRadians(90)); // 绕 Z 轴旋转 90°

// 旋转组合（先 Q1 后 Q2）
FQuat Combined = Q2 * Q1;   // 注意顺序！右乘 = 先应用左边

// 插值（动画混合的核心）
float Alpha = 0.5f;  // 0.0=A, 1.0=B
FQuat Blended = FQuat::Slerp(QA, QB, Alpha);  // 球形线性插值（精确）
FQuat Blended2 = FQuat::FastLerp(QA, QB, Alpha); // 快速近似（性能更好）
FQuat Blended3 = FQuat::Slerp_NotNormalized(QA, QB, Alpha); // 不规范化版本

// 获取旋转角度和轴
float Angle;
FVector Axis;
Q.ToAxisAndAngle(Axis, Angle);

// 转换为欧拉角（小心万向节锁！）
FRotator Euler = Q.Rotator();

// 规范化（确保为单位四元数）
Q.Normalize();
FQuat NQ = Q.GetNormalized();

// 反转旋转
FQuat Inverse = Q.Inverse();
```

---

## 六、坐标系常见陷阱

### 陷阱 1：从 Maya/Blender 导入轴向错误

```
问题：导入的角色朝向错误，或骨骼旋转轴与预期不符

根本原因：
  Maya 默认 Y-Up，UE 是 Z-Up
  Maya 的 Z-Forward 变为 UE 的 X-Forward

解决方案（推荐）：
  在 Maya 中：File → Export → FBX Settings → Up Axis = Z
  这样 UE 导入时无需额外旋转补偿
  
解决方案（补救）：
  UE 导入设置 → Transform → Import Rotation X = -90
  注意：这只影响 Mesh，不影响动画，可能需要分别设置
```

### 陷阱 2：AnimGraph 中误用 World Space

```
问题：在 AnimNode 中直接使用 Actor 的世界坐标进行骨骼控制

正确做法：
  将世界坐标转换为 Component Space 后再传给骨骼控制节点

FVector WorldTarget = ...; // 目标点的世界坐标

// 转换为 Component Space
FVector ComponentTarget = SkeletalMesh->GetComponentTransform()
    .InverseTransformPosition(WorldTarget);

// 传给 IK 节点的效应器目标
IKEffectorLocation = ComponentTarget;
```

### 陷阱 3：骨骼方向与蒙皮不匹配

```
问题：修改骨骼旋转后，网格体扭曲变形

原因：骨骼的"朝向"（Roll 角）会影响蒙皮，
      如果骨骼旋转导致其局部 X 轴方向改变，
      蒙皮权重的映射就会出错

解决：在 DCC 中建立正确的骨骼朝向，导入前不要在 UE 中强行旋转骨骼
```

---

## 七、延伸阅读

- 🔗 [Unreal 骨骼动画源码剖析（知乎）](https://zhiruili.github.io/posts/unreal-skeletal-animation-source-code/)
- 🔗 [UE5 动画源码解析（知乎）](https://zhuanlan.zhihu.com/p/1927668362969880273)
- 📄 [代码示例：骨骼查询工具](../code/02_skeleton_query.cpp)
