#pragma once

#include "CoreMinimal.h"
#include "FlowField.h"
#include "Navigation.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FNavigation
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "流场"))
	TSoftObjectPtr<AFlowField> FlowFieldToUse;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "更换流场后要设该值为true来通知更新数据"))
	bool bIsDirtyData = true;

	AFlowField* FlowField = nullptr;
};
