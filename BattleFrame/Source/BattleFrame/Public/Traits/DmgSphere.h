#pragma once

#include "CoreMinimal.h"
#include "Traits/Damage.h"
#include "DmgSphere.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDmgSphere
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "伤害值"))
	float Damage = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "伤害类型"))
	EDmgType DmgType = EDmgType::Normal;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "百分比伤害"))
	float PercentDmg = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "暴击倍数"))
	float CritDmgMult = 2.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "暴击概率"))
	float CritProbability = 0.1;

};
