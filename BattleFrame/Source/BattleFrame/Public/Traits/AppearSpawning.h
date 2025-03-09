#pragma once

#include "CoreMinimal.h"
#include "AppearSpawning.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FAppearSpawning
{
	GENERATED_BODY()

public:

	TSubclassOf<AActor> AppearDecalClass = nullptr;

	FAppearSpawning() {};
};
