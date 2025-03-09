#pragma once

#include "CoreMinimal.h"
#include "Defence.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDefence
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用防御属性"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "对普通伤害的免疫比例（0-1）"))
	float NormalDmgImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "对动能伤害的免疫比例（0-1）"))
	float KineticDmgImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "对动能Debuff的免疫比例（0-1）"))
	float KineticDebuffImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "对火焰伤害的免疫比例（0-1）"))
	float FireDmgImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "对火焰Debuff的免疫比例（0-1）"))
	float FireDebuffImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "对冰冻伤害的免疫比例（0-1）"))
	float IceDmgImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "对冰冻Debuff的免疫比例（0-1）"))
	float IceDebuffImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "对百分比伤害的免疫比例（0-1）"))
	float PercentDmgImmune = 0.f;
};
