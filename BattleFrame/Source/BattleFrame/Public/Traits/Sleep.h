#pragma once

#include "CoreMinimal.h"
#include "Sleep.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSleep
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool enableGlow = true;

};