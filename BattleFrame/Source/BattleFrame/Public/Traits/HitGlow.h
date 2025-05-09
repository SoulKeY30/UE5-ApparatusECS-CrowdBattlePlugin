#pragma once
 
#include "CoreMinimal.h"
#include "HitGlow.generated.h"
 
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FHitGlow
{
	GENERATED_BODY()
 
  public:

	float GlowTime = 0.0f;
	bool bGlowFinished = false;

};
