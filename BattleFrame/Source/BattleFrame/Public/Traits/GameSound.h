#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundBase.h"
#include "GameSound.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FGameSound
{
	GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    USoundBase* WaveStartSound = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    USoundBase* WaveClearSound = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    USoundBase* WaveFailSound = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    USoundBase* RestartSound = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    USoundBase* GameOverSound = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    USoundBase* ClearAllLevelSound = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    USoundBase* ShieldOnSound = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    USoundBase* ShieldOffSound = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
    USoundBase* RelicAddHealthSound = nullptr;

};