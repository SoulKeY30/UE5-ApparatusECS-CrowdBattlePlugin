#pragma once

#include "CoreMinimal.h"
#include "Escape.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FEscape
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bEnable = true;

};