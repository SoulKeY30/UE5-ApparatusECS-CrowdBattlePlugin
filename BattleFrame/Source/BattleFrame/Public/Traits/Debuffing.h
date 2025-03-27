#pragma once
 
#include "CoreMinimal.h"
#include "Debuffing.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDebuffing
{
	GENERATED_BODY()

public:

	float FreezeTimeout = 4.f;

	float OriginalDecoupleProportion = 1.f;

};