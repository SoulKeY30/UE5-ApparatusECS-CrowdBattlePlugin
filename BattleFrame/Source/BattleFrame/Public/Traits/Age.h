#pragma once
 
#include "CoreMinimal.h"
#include "Age.generated.h"
 
/**
 * The main enemy trait.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FAge
{
	GENERATED_BODY()
 
  public:

	float Age = 0;
};
