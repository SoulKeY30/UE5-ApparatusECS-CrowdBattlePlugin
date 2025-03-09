#pragma once

#include "CoreMinimal.h"
#include "DeathSpawning.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDeathSpawning
{
	GENERATED_BODY()

public:

    TSubclassOf<AActor> DeathSpawnClass = nullptr;
    int32 DeathSpawnQuantity = 0;
};
