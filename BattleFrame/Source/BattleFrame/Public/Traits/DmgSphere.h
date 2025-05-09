#pragma once

#include "CoreMinimal.h"
#include "DmgSphere.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDmgSphere
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "普通伤害，表示兵种的基础伤害值"))
	float Damage = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "动能伤害，表示兵种对目标造成的动能类型伤害"))
	float KineticDmg = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "火焰伤害，表示兵种对目标造成的火焰类型伤害"))
	float FireDmg = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "冰冻伤害，表示兵种对目标造成的冰冻类型伤害"))
	float IceDmg = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "百分比伤害，表示兵种对目标造成的基于目标最大生命值的百分比伤害"))
	float PercentDmg = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "暴击概率，表示兵种攻击时触发暴击的概率"))
	float CritProbability = 0.1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "暴击倍率，表示兵种触发暴击时的伤害倍率"))
	float CritMult = 2.f;

};
