#pragma once

#include "CoreMinimal.h"
#include "SpawningProjectile.generated.h"

/**
 * The main projectile trait.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSpawningProjectile
{
	GENERATED_BODY()

public:

	TSubclassOf<AActor> ProjectileClass = nullptr;

};
