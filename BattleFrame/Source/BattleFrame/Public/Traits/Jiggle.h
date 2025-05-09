#pragma once

#include "CoreMinimal.h"
#include "Jiggle.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FJiggle
{
	GENERATED_BODY()

public:

	float JiggleTime = 0.0f;
	bool JiggleFinished = false;

};
