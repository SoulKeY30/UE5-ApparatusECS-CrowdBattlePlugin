#pragma once
 
#include "CoreMinimal.h"
#include "Appearing.generated.h"
 
/**
 * The state of appearing in game.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FAppearing
{
	GENERATED_BODY()
 
  public:

	  float time = 0.0f;

	  FAppearing() {};
};
