# 9.1 网络架构基础与动画同步原理

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、UE 网络架构回顾

```
UE 采用 Server-Authority（服务端权威）模型：

                     ┌──────────────┐
                     │    Server    │  ← 游戏逻辑真相来源
                     └──────┬───────┘
                    RPC/Rep  │
          ┌──────────────────┼──────────────────┐
          ▼                  ▼                  ▼
   ┌────────────┐    ┌────────────┐    ┌────────────┐
   │  Client 1  │    │  Client 2  │    │  Client 3  │
   │ (AutProxy) │    │ (SimProxy) │    │ (SimProxy) │
   └────────────┘    └────────────┘    └────────────┘

角色扮演：
  Server           ：完整游戏逻辑，拥有所有 Actor 的权威状态
  AutonomousProxy  ：本地玩家控制的角色（Client 1 视角）
  SimulatedProxy   ：其他玩家的角色（Client 1 看到的 Client 2/3）
```

---

## 二、动画同步的核心挑战

```
问题：动画状态不直接复制（太贵！）

若直接复制所有骨骼变换（200骨骼 × float3 × 60fps）：
  = 200 × 12 bytes × 60 = ~144KB/s / 角色
  10 个角色 = 1.44MB/s → 完全不可行

实际做法：复制"驱动动画的状态变量"（而非动画本身）

可复制的内容                    带宽
─────────────────────────────────────────────────────
移动速度（FVector，压缩）       ~3 bytes
是否在空中（bool）              ~1 bit
当前 Montage（uint8 index）    ~1 byte
受击方向（FRotator，压缩）      ~2 bytes

总计：< 10 bytes/帧 → 可接受
```

---

## 三、动画同步的三种机制

```
机制 1：CharacterMovement 自动同步（最重要）
  UE 内置，自动处理位置/旋转/速度同步
  AnimBP 读取 GetVelocity() 就能得到正确值
  覆盖：行走/跑步/跳跃/落地等基础移动动画

机制 2：复制属性（Replicated Properties）
  在 Character C++ 中声明 UPROPERTY(Replicated)
  服务端修改 → 自动同步到所有客户端
  AnimBP 读取这些变量驱动动画状态机
  覆盖：姿势状态（战斗/非战斗）、武器类型、特殊状态

机制 3：RPC（Remote Procedure Call）
  客户端 → 服务端（ServerRPC）：触发技能/攻击
  服务端 → 所有客户端（NetMulticast）：播放特效/Montage
  覆盖：一次性动画事件（攻击/受击/技能释放）
```

---

## 四、动画角色分类处理

```
同一个 AnimBlueprint，对不同角色角色扮演分支处理：

// AnimBP EventGraph 中
if (OwnerCharacter->IsLocallyControlled())
{
    // AutonomousProxy：直接读取本地输入状态（最流畅）
    Speed = OwnerCharacter->GetVelocity().Size2D();
}
else
{
    // SimulatedProxy：读取从服务端同步来的复制变量
    Speed = OwnerCharacter->ReplicatedSpeed; // 经过网络同步的版本
}

为什么分开处理？
  本地玩家：直接读本地数据（零延迟，最流畅）
  其他玩家：必须读同步数据（否则会用本地预测的错误值）
```

---

## 五、延伸阅读

- 📄 [9.2 移动同步详解](./02-movement-replication.md)
- 🔗 [网络概述文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/networking-overview-for-unreal-engine)
- 🔗 [CharacterMovement 网络](https://dev.epicgames.com/documentation/en-us/unreal-engine/understanding-networked-movement-in-the-character-movement-component)
