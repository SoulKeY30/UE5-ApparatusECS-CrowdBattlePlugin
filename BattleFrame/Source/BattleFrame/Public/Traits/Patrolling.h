#pragma once

#include "CoreMinimal.h"
#include "Patrolling.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FPatrolling
{
	GENERATED_BODY()

public:

	float MoveTimeLeft = 0;
	float WaitTimeLeft = 0;
	FVector GoalLocation = FVector::ZeroVector;
};