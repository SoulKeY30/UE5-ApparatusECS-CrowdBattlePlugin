#pragma once

#include "CoreMinimal.h"
#include "FlowField.h"
#include "Navigation.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FNavigation
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Navigation", meta = (Tooltip = "流场Actor的软引用，用于导航"))
	TSoftObjectPtr<AFlowField> FlowFieldActor;

	AFlowField* FlowField = nullptr;
};
