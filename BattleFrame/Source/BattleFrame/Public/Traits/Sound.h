#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundBase.h"
#include "Sound.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSound
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否使用概率控制音效播放"))
    bool bUseProbability = false;

    // Appear
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "出生音效的软引用"))
    TSoftObjectPtr<USoundBase> AppearSound = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "出生音效的概率参数 (X: 最小距离, Y: 最大距离, Z: 最小概率, W: 最大概率)"))
    FVector4 AppearSoundProbability = FVector4(1, 2000, 1, 0.01);

    // Attack
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "攻击音效的软引用"))
    TSoftObjectPtr<USoundBase> AttackSound = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "攻击音效的概率参数 (X: 最小距离, Y: 最大距离, Z: 最小概率, W: 最大概率)"))
    FVector4 AttackSoundProbability = FVector4(1, 2000, 1, 0.01);

    // Hit
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "受击音效的软引用"))
    TSoftObjectPtr<USoundBase> HitSound = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "受击音效的概率参数 (X: 最小距离, Y: 最大距离, Z: 最小概率, W: 最大概率)"))
    FVector4 HitSoundProbability = FVector4(1, 2000, 1, 0.01);

    // Death
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "死亡音效的软引用"))
    TSoftObjectPtr<USoundBase> DeathSound = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "死亡音效的概率参数 (X: 最小距离, Y: 最大距离, Z: 最小概率, W: 最大概率)"))
    FVector4 DeathSoundProbability = FVector4(1, 2000, 1, 0.01);
};
