 /*
  * BattleFrame
  * Refactor: 2025
  * Author: Leroy Works
  */

#pragma once

#include "CoreMinimal.h"
#include "Directed.generated.h"


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDirected
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Direction = FVector::ForwardVector;

	FVector DesiredDirection = FVector::ForwardVector;

};