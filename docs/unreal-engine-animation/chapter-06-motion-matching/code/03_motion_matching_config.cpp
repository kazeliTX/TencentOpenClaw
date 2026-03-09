// =============================================================================
// 03_motion_matching_config.cpp
// Motion Matching 数据库 C++ 辅助工具
// 对应文档：chapter-06-motion-matching/07-mm-database.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "PoseSearch/PoseSearchDatabase.h"

/**
 * Motion Matching 运行时数据库切换辅助类
 * 根据角色状态动态切换 Pose Search Database
 */
UENUM(BlueprintType)
enum class ELocomotionMode : uint8
{
    Ground   UMETA(DisplayName="Ground"),
    Combat   UMETA(DisplayName="Combat"),
    Crouch   UMETA(DisplayName="Crouch"),
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API UMotionMatchingDatabaseSelector : public UActorComponent
{
    GENERATED_BODY()
public:
    // 各模式数据库（编辑器中配置）
    UPROPERTY(EditAnywhere, Category="MM") TObjectPtr<UPoseSearchDatabase> GroundDB;
    UPROPERTY(EditAnywhere, Category="MM") TObjectPtr<UPoseSearchDatabase> CombatDB;
    UPROPERTY(EditAnywhere, Category="MM") TObjectPtr<UPoseSearchDatabase> CrouchDB;

    // 当前数据库（AnimGraph 读取此变量）
    UPROPERTY(BlueprintReadOnly, Category="MM")
    TObjectPtr<UPoseSearchDatabase> ActiveDatabase;

    UFUNCTION(BlueprintCallable, Category="MM")
    void SetLocomotionMode(ELocomotionMode Mode)
    {
        if (CurrentMode == Mode) return;
        CurrentMode = Mode;
        switch (Mode)
        {
            case ELocomotionMode::Ground: ActiveDatabase = GroundDB; break;
            case ELocomotionMode::Combat: ActiveDatabase = CombatDB; break;
            case ELocomotionMode::Crouch: ActiveDatabase = CrouchDB; break;
        }
    }

    UFUNCTION(BlueprintPure) ELocomotionMode GetLocomotionMode() const { return CurrentMode; }

private:
    ELocomotionMode CurrentMode = ELocomotionMode::Ground;
};

// =============================================================================
// AnimBP 中：Motion Matching 节点 → Database = DatabaseSelector.ActiveDatabase
// （通过变量绑定，切换模式时自动切换数据库，无需重建 MM 节点）
// =============================================================================
