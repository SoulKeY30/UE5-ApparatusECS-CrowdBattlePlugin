#pragma once

#include "CoreMinimal.h"
#include "Chase.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FChase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "启用追逐行为"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "是否索敌"))
	bool bCanTrace = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "与攻击目标距离低于该值时结束追逐"))
	float AcceptanceRadius = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "移动速度乘数"))
	float MoveSpeedMult = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "距离超过该值丢失仇恨"))
	float MaxDistance = 10000;
};
