// Turbanov Toolworks 2023

#pragma once

#include "CoreMinimal.h"
#include "Gravity.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FGravity
{
	GENERATED_BODY()

public:
	/* ����*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Gravity = 4000.f;
};
