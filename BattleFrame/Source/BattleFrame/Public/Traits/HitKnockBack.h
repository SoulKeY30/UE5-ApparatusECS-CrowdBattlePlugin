#pragma once
 
#include "CoreMinimal.h"
#include "HitKnockBack.generated.h"
 
/**
 * The state of being hit by a projectile.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FHitKnockBack
{
	GENERATED_BODY()
 
  public:

	/**
	 * The velocity to knock back with.
	 */
	FVector Velocity = FVector::ZeroVector;

};
