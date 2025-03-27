#pragma once
 
#include "CoreMinimal.h"
#include "Freezing.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FFreezing
{
	GENERATED_BODY()

public:

	float FreezeTimeout = 4.f;
	float FreezeStr = 1.f;
	float OriginalDecoupleProportion = 1.f;

};