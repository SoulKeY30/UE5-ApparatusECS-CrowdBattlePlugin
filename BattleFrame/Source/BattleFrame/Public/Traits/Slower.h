#pragma once
 
#include "CoreMinimal.h"
#include "Traits/Damage.h"
#include "Slower.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSlower
{
	GENERATED_BODY()

public:

	FSubjectHandle SlowTarget = FSubjectHandle();

	float SlowTimeout = 4.f;
	float SlowStrength = 1.f;
	EDmgType DmgType = EDmgType::Normal;
	bool bJustSpawned = true;
};