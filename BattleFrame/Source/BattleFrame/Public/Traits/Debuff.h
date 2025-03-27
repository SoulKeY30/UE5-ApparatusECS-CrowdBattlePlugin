#pragma once

#include "CoreMinimal.h"
#include "Debuff.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDebuff
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用减益效果"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否可以将目标击退"))
	bool bCanKnockback = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "击退的速度与方向"))
	float KnockbackSpeed = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否可以造成延时伤害"))
	bool bCanBurn = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "延时伤害占直接伤害的百分比"))
	float BurnDmgRatio = 0.25f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否可以使目标减速"))
	bool bCanFreeze = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "减速的持续时间（秒）"))
	float FreezeTime = 4.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "减速的强度"))
	float FreezeStr = 1.f;

};
