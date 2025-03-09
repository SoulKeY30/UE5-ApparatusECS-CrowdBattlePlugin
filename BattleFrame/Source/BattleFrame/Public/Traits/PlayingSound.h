#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundBase.h"
#include "PlayingSound.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FPlayingSound
{
    GENERATED_BODY()

public:
    // Array to store the sounds to be played
    USoundBase* Sound = nullptr;

    // Array to store the locations where the sounds should be played
    FVector Location = FVector::ZeroVector;

    float Volume = 1;

    float Age = 1.f;

};
