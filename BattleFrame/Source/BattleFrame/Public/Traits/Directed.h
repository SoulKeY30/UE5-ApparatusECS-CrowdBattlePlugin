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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "当钱朝向"))
	FVector Direction = FVector::ForwardVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "目标朝向"))
	FVector DesiredDirection = FVector::ForwardVector;

};