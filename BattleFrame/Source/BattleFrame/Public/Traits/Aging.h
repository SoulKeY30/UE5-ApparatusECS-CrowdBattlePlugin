#pragma once
 
#include "CoreMinimal.h"
#include "Aging.generated.h"
 
/**
 * The main enemy trait.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FAging
{
	GENERATED_BODY()
 
  public:

	float Age = 0;
};
