#pragma once

#include "CoreMinimal.h"
#include "Damage.generated.h"

UENUM(BlueprintType)
enum class EDmgType : uint8
{
	Normal UMETA(DisplayName = "Normal Damage", Tooltip = "普通伤"),
	Fire UMETA(DisplayName = "Fire Damage", Tooltip = "火伤"),
	Ice UMETA(DisplayName = "Ice Damage", Tooltip = "冰伤"),
	Poison UMETA(DisplayName = "Poison Damage", Tooltip = "毒伤")
	//Heal UMETA(DisplayName = "Heal", Tooltip = "治疗")
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDamage
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "普通伤"))
	float Damage = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "伤害类型"))
	EDmgType DmgType = EDmgType::Normal;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "百分比伤"))
	float PercentDmg = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "暴击倍数"))
	float CritDmgMult = 2.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "暴击概率"))
	float CritProbability = 0.1f;

};
