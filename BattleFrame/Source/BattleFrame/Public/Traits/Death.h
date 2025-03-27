#pragma once

#include "CoreMinimal.h"
#include "SubType.h"
#include "Death.generated.h"

/**
 * The state of dying.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDeath
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用死亡逻辑"))
    bool bEnable = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "死亡后消失的延迟时间（单位：秒）"))
    float DespawnDelay = 3.f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用淡出效果"))
    bool bCanFadeout = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "淡出效果的延迟时间（单位：秒）"))
    float FadeOutDelay = 2.f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否播放死亡动画"))
    bool bCanPlayAnim = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "死亡时生成的战利品数量"))
    int32 NumSpawnLoot = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否播放死亡特效"))
    bool bCanSpawnFx = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否播放死亡音效"))
    bool bCanPlaySound = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "死亡后的尸体是否关闭碰撞"))
    bool bDisableCollision = true;

    bool bDeathStarted = false;
};
