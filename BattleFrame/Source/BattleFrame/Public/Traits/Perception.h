#pragma once

#include "CoreMinimal.h"
#include "Perception.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FPerception
{
	GENERATED_BODY()

public:

	bool bCanSee = false;
	bool bCanHear = false;
	bool bCanFeel = false;
};