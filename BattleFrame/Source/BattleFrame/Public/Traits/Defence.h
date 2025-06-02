#pragma once

#include "CoreMinimal.h"
#include "Defence.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDefence
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用抗性"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "对普通伤害的免疫比例（0-1）"))
	float NormalDmgImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "对火焰伤害的免疫比例（0-1）"))
	float FireDmgImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "对冰冻伤害的免疫比例（0-1）"))
	float IceDmgImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "对动能伤害的免疫比例（0-1）"))
	float PoisonDmgImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "对百分比伤害的免疫比例（0-1）"))
	float PercentDmgImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "存活时对动能Debuff的免疫比例（0-1）"))
	float LaunchImmuneAlive = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "死亡后对动能Debuff的免疫比例（0-1）"))
	float LaunchImmuneDead = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "最大被击退力"))
	float LaunchMaxImpulse = 10000.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "对冰冻Debuff的免疫比例（0-1）"))
	float SlowImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "被冰冻是否减攻击速度"))
	bool bCanSlowATKSpeed = false;



};
