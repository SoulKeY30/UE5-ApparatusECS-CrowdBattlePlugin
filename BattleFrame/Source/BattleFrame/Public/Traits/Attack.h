#pragma once

#include "CoreMinimal.h"
#include "SubType.h"
#include "Attack.generated.h"

UENUM(BlueprintType)
enum class EAttackMode : uint8
{
	Suicide UMETA(DisplayName = "Suicide", Tooltip = "造成伤害后自毁"),
	Melee UMETA(DisplayName = "Melee", Tooltip = "在TimeOfAttack造成伤害"),
	Ranged UMETA(DisplayName = "Ranged", Tooltip = "在TimeOfAttack生成Actor")
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FAttack
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用攻击功能"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "攻击模式 (Suicide: 自杀式, Melee: 近战, Ranged: 远程)"))
	EAttackMode AttackMode = EAttackMode::Melee;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "攻击范围（单位：厘米）"))
	float Range = 200.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "近战攻击能造成伤害的角度范围"))
	float MeleeAngle = 60.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "施加伤害/生成子弹的时刻（单位：秒）"))
	float TimeOfAttack = 0.35f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "每轮攻击的持续时间（单位：秒）"))
	float DurationPerRound = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "攻击冷却时间（单位：秒）"))
	float CoolDown = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否播放攻击动画"))
	bool bCanPlayAnim = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否生成危险警告"))
	bool bCanSpawnDangeWarning = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否生成危险警告"))
	bool bCanSpawnProjectile = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否播放攻击特效"))
	bool bCanSpawnFx = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否播放攻击音效"))
	bool bCanPlaySound = true;
};
