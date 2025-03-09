#pragma once

#include "CoreMinimal.h"
#include "Patrol.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FPatrol
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool enableGlow = true;

};