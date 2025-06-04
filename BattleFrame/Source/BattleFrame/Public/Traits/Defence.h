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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "普通伤害免疫比例（0-1）"))
	float NormalDmgImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "火焰伤害免疫比例（0-1）"))
	float FireDmgImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "冰冻伤害免疫比例（0-1）"))
	float IceDmgImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "毒伤免疫比例（0-1）"))
	float PoisonDmgImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "百分比伤害免疫比例（0-1）"))
	float PercentDmgImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "存活时对击飞的免疫比例（0-1）"))
	float LaunchImmuneAlive = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "死亡后对击飞的免疫比例（0-1）"))
	float LaunchImmuneDead = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "最大可积累被击飞力"))
	float LaunchMaxImpulse = 10000.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "对减速的免疫比例（0-1）"))
	float SlowImmune = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "被减速时是否减攻击速度"))
	bool bCanSlowATKSpeed = false;

};
