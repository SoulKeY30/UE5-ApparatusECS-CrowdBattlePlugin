#pragma once

#include "CoreMinimal.h"
#include "Damage.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDamage
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "基础伤害值"))
	float Damage = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "动能伤害值"))
	float KineticDmg = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "火焰伤害值"))
	float FireDmg = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "冰冻伤害值"))
	float IceDmg = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "百分比伤害比例"))
	float PercentDmg = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "暴击概率"))
	float CritProbability = 0.1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "暴击伤害倍数"))
	float CritMult = 2.f;

};
