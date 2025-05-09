#pragma once
 
#include "CoreMinimal.h"
#include "HitAnim.generated.h"
 
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FHitAnim
{
	GENERATED_BODY()
 
  public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool enableAnim = false;

	float animTime = 0.0f;
	bool animFinished = false;

};
