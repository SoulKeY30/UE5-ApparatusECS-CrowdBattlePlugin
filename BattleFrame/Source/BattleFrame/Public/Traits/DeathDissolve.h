#pragma once
 
#include "CoreMinimal.h"
#include "DeathDissolve.generated.h"
 
/**
 * The state of being hit by a projectile.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDeathDissolve
{
	GENERATED_BODY()
 
  public:

	float dissolveTime = 0.0f;
};
