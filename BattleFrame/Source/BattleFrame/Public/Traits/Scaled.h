#pragma once

#include "CoreMinimal.h"
#include "Scaled.generated.h"


USTRUCT(BlueprintType, Category = "Basic")
struct BATTLEFRAME_API FScaled
{
	GENERATED_BODY()

  public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "实际尺寸"))
	float Scale = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "模型渲染尺寸,Jiggle逻辑靠修改这个值来起作用"))
	FVector RenderScale = FVector::OneVector;

};
