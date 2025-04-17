#pragma once

#include "CoreMinimal.h"
#include "Patrolling.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FPatrolling
{
	GENERATED_BODY()

public:

	float TimeLeft = 0;

	FVector TargetLocation = FVector::ZeroVector;
};