#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundBase.h"
#include "MonsterSound.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FMonsterSound
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool useSoundProbability = true;

    // Appear
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TSoftObjectPtr<USoundBase> appearSound = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector4 appearSoundProbability = FVector4(1, 2000, 1, 0.01);

    // Attack
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TSoftObjectPtr<USoundBase> attackSound = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector4 attackSoundProbability = FVector4(1, 2000, 1, 0.01);

    // Hit
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TSoftObjectPtr<USoundBase> hitSound = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector4 hitSoundProbability = FVector4(1, 2000, 1, 0.01);

    // Death
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TSoftObjectPtr<USoundBase> deathSound = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector4 deathSoundProbability = FVector4(1, 2000, 1, 0.01);

};