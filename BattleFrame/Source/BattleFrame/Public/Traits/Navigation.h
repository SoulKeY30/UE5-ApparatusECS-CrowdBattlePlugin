#pragma once

#include "CoreMinimal.h"
#include "FlowField.h"
#include "Navigation.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FNavigation
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Navigation", meta = (Tooltip = "没有目标时按该流场移动"))
	TSoftObjectPtr<AFlowField> FlowFieldToUse;

	TSoftObjectPtr<AFlowField> PreviousFlowFieldToUse;

	AFlowField* FlowField = nullptr;
};
