#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundBase.h"
#include "Sound.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSound
{
    GENERATED_BODY()

public:

    // Appear
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "出生音效的软引用"))
    TSoftObjectPtr<USoundBase> AppearSound = nullptr;

    // Attack
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "攻击音效的软引用"))
    TSoftObjectPtr<USoundBase> AttackSound = nullptr;

    // Hit
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "受击音效的软引用"))
    TSoftObjectPtr<USoundBase> HitSound = nullptr;

    // Death
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "死亡音效的软引用"))
    TSoftObjectPtr<USoundBase> DeathSound = nullptr;

};
