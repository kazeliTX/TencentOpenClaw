// =============================================================================
// 02_montage_replication.cpp
// Montage 多播同步系统（客户端预测 + 服务端确认）
// 对应文档：chapter-09-network/04-montage-replication.md
// =============================================================================
#pragma once

// ── 在 MyCharacter.h 中声明 ──────────────────────────────────────────────────
// UFUNCTION(Server, Reliable)   void ServerPlayMontageByID(uint8 ID);
// UFUNCTION(NetMulticast, Reliable) void MulticastPlayMontageByID(uint8 ID);
// UFUNCTION(NetMulticast, Reliable) void MulticastStopAllMontages(float BlendOut);
// ─────────────────────────────────────────────────────────────────────────────

// ── MyCharacter.cpp 实现 ──────────────────────────────────────────────────────

// 客户端请求播放（本地预测 + RPC）
void AMyCharacter::RequestPlayMontage(uint8 MontageID)
{
    if (!MontageList.IsValidIndex(MontageID)) return;

    // 本地立即播放（预测，无延迟）
    if (IsLocallyControlled())
        PlayAnimMontage(MontageList[MontageID]);

    // 告知服务端
    if (!HasAuthority())
        ServerPlayMontageByID(MontageID);
    else
        MulticastPlayMontageByID(MontageID); // 服务端直接广播
}

void AMyCharacter::ServerPlayMontageByID_Implementation(uint8 MontageID)
{
    if (!MontageList.IsValidIndex(MontageID)) return;
    // 服务端验证（可添加冷却/状态检查）
    // if (!CanExecuteAction()) { ClientCancelMontage(); return; }

    PlayAnimMontage(MontageList[MontageID]); // 服务端自己播放
    MulticastPlayMontageByID(MontageID);     // 广播给其他客户端
}

void AMyCharacter::MulticastPlayMontageByID_Implementation(uint8 MontageID)
{
    // 跳过本地控制的客户端（已预测播放）
    if (!IsLocallyControlled() && MontageList.IsValidIndex(MontageID))
        PlayAnimMontage(MontageList[MontageID]);
}

void AMyCharacter::MulticastStopAllMontages_Implementation(float BlendOut)
{
    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
        Anim->Montage_StopAll(BlendOut);
}
