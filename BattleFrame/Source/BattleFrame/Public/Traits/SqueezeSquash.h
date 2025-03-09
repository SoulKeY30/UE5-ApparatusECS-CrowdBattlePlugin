#pragma once

#include "CoreMinimal.h"
#include "SqueezeSquash.generated.h"

/**
 * The state of being hit by a projectile.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSqueezeSquash
{
	GENERATED_BODY()

public:

	float squeezeSquashTime = 0.0f;
	bool squeezeSquashFinished = false;

};
