#pragma once

#include "CoreMinimal.h"
#include "FlowField.h"
#include "BindFlowField.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FBindFlowField
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Navigation", meta = (Tooltip = "指向自己的流场"))
	TSoftObjectPtr<AFlowField> FlowFieldToBind;

	TSoftObjectPtr<AFlowField> PreviousFlowFieldToBind;

	AFlowField* FlowField = nullptr;
};
