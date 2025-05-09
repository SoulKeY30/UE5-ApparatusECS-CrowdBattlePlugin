#pragma once
 
#include "CoreMinimal.h"
#include "DeathAnim.generated.h"
 
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDeathAnim
{
	GENERATED_BODY()
 
  public:

	bool enableAnim = false;
	float animTime = 0.0f;
	bool animFinished = false;

};
