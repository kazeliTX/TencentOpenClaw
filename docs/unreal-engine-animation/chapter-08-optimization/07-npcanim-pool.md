# 8.7 NPC 动画池与实例复用

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、同类 NPC 动画复用

```
问题：场景中 50 个同类守卫，每个都有独立 AnimInstance
  → 50 个 AnimInstance，50 份独立计算

优化思路：同类 NPC 共享动画数据

方案 A：Copy Pose（姿势复制）
  指定一个"主 NPC"全速计算动画
  其他同类 NPC：AnimGraph 中 Copy Pose from Mesh
  → 从主 NPC 的 Mesh 复制骨骼变换
  → 多个 NPC 共享同一份动画计算结果

方案 B：Shared Animation（AnimInstance 共享）
  让多个 SkeletalMeshComponent 使用同一个 AnimInstance
  → 完全共享，开销降至 1/N
  → 但所有 NPC 姿势完全相同（适合人群/背景角色）

方案 C：Animation 时间偏移
  所有 NPC 播放相同动画，但有随机时间偏移
  → 看起来不完全同步，有自然感
  → 零额外计算开销
```

---

## 二、Copy Pose 实现

```
AnimGraph 中使用 Copy Pose from Mesh 节点：

配置：
  Source Mesh Component: 引用"主 NPC"的 SkeletalMeshComponent
  Copy Curves: ✅（可选，复制动画曲线数据）
  
主 NPC 选择策略：
  距离玩家最近的同类 NPC 作为主 NPC
  主 NPC 全速更新（LOD0，60fps）
  其他 NPC：Copy Pose（极低开销）
  
注意：
  所有使用 Copy Pose 的 NPC 姿势完全相同
  → 只适合远处的"群众演员"
  近处可交互的 NPC 仍需独立 AnimInstance
```

---

## 三、NPC 动画池管理器

```cpp
// 简单的 NPC 动画池（将相似 NPC 分组，组内复用姿势）
class FNPCAnimPool
{
public:
    // 注册一个 NPC 到动画池
    void RegisterNPC(ACharacter* NPC, FName GroupName)
    {
        FNPCGroup& Group = Groups.FindOrAdd(GroupName);
        Group.Members.Add(NPC);
        // 按距离玩家排序（最近的成为主 NPC）
        SortGroup(Group);
    }

    // 每帧：更新主 NPC 并让其他 NPC Copy Pose
    void Tick(float DeltaTime, FVector PlayerLocation)
    {
        for (auto& [Name, Group] : Groups)
        {
            if (Group.Members.IsEmpty()) continue;

            // 找到最近的有效成员作为主 NPC
            ACharacter* Primary = FindClosestMember(Group, PlayerLocation);
            if (!Primary) continue;

            // 主 NPC：恢复独立 AnimInstance
            USkeletalMeshComponent* PrimaryMesh = Primary->GetMesh();
            PrimaryMesh->SetComponentTickEnabled(true);

            // 其他 NPC：降频或 Copy Pose
            for (ACharacter* Member : Group.Members)
            {
                if (Member == Primary) continue;
                float Dist = FVector::Dist(Member->GetActorLocation(), PlayerLocation);
                USkeletalMeshComponent* Mesh = Member->GetMesh();
                
                if (Dist > 30.f * 100.f) // > 30m：Copy Pose 或停止
                    Mesh->SetComponentTickInterval(1.f / 5.f); // 5fps
                else
                    Mesh->SetComponentTickInterval(1.f / 15.f); // 15fps
            }
        }
    }

private:
    struct FNPCGroup { TArray<ACharacter*> Members; };
    TMap<FName, FNPCGroup> Groups;

    ACharacter* FindClosestMember(FNPCGroup& Group, FVector Pos)
    {
        ACharacter* Closest = nullptr;
        float MinDist = FLT_MAX;
        for (ACharacter* C : Group.Members)
        {
            float D = FVector::DistSquared(C->GetActorLocation(), Pos);
            if (D < MinDist) { MinDist = D; Closest = C; }
        }
        return Closest;
    }
};
```

---

## 四、延伸阅读

- 📄 [代码示例：人群管理器](./code/03_crowd_manager.cpp)
- 🔗 [Copy Pose 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-optimization-in-unreal-engine)
