#pragma once

#include "CoreMinimal.h"
#include "SubType.h"
#include "ActorSpawnConfig.h"
#include "SoundConfig.h"
#include "FxConfig.h"
#include "Attack.generated.h"

UENUM(BlueprintType)
enum class EAttackMode : uint8
{
	None UMETA(DisplayName = "None", Tooltip = "无"),
	ApplyDMG UMETA(DisplayName = "Apply Damage", Tooltip = "造成伤害"),
	SuicideATK UMETA(DisplayName = "Apply Damage And Despawn", Tooltip = "造成伤害后自毁")
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FAttack
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用攻击功能"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "距离小于该值可以攻击"))
	float Range = 200.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "夹角小于该值可以攻击"))
	float AngleTolerance = 60.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "每轮攻击的持续时长"))
	float DurationPerRound = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "施加伤害的时刻"))
	float TimeOfHit = 0.35f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "施加伤害的时刻的行为"))
	EAttackMode TimeOfHitAction = EAttackMode::ApplyDMG;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "攻击冷却时长"))
	float CoolDown = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否播放攻击动画"))
	bool bCanPlayAnim = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	TArray<FActorSpawnConfig> SpawnActor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	TArray<FFxConfig> SpawnFx;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	TArray<FSoundConfig> PlaySound;

};
