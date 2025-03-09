#pragma once

#include "CoreMinimal.h"
#include "SpawningActor.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSpawningActor
{
	GENERATED_BODY()

public:

	TSubclassOf<AActor> SpawnClass = nullptr;
	int32 Quantity = 1;
	FTransform Trans;
};
