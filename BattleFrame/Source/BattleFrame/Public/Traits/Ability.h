#pragma once
 
#include "CoreMinimal.h"
#include "Ability.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FAbility
{
	GENERATED_BODY()

public:

	int32 AbilityType = 0;
};