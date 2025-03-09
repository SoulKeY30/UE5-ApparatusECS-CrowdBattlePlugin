#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundBase.h"
#include "TowerSound.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTowerSound
{
	GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
     bool useSoundProbability = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    TSoftObjectPtr<USoundBase> ShootSoundNormal = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
        FVector4 ShootSoundNormalProbability = FVector4(1,100,1,0.05);

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    TSoftObjectPtr<USoundBase> ShootSoundSkill1 = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    FVector4 ShootSoundSkill1Probability = FVector4(1,100,1,0.05);

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    TSoftObjectPtr<USoundBase> ShootSoundSkill2 = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
        FVector4 ShootSoundSkill2Probability = FVector4(1,100,1,0.05);

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    TSoftObjectPtr<USoundBase> HitSoundNormal = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    FVector4 HitSoundNormalProbability = FVector4(1,100,1,0.05);

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    TSoftObjectPtr<USoundBase> HitSoundSkill1 = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
        FVector4 HitSoundSkill1Probability = FVector4(1,100,1,0.02);

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    TSoftObjectPtr<USoundBase> HitSoundSkill2 = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
        FVector4 HitSoundSkill2Probability = FVector4(1,100,1,0.01);

};
