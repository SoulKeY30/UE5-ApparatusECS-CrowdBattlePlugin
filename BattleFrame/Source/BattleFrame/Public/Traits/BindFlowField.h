#pragma once

#include "CoreMinimal.h"
#include "FlowField.h"
#include "BindFlowField.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FBindFlowField
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "流场"))
	TSoftObjectPtr<AFlowField> FlowFieldToBind;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "更换流场后要设该值为true来通知更新数据"))
	bool bIsDirtyData = true;

	AFlowField* FlowField = nullptr;

};
