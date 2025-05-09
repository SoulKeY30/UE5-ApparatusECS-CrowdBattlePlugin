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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	FVector Direction = FVector::ForwardVector;

	FRotator DesiredRot = FRotator::ZeroRotator;

};