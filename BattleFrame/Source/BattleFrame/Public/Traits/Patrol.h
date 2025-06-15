#pragma once

#include "CoreMinimal.h"
#include "BattleFrameStructs.h"
#include "Patrol.generated.h" 

UENUM(BlueprintType)
enum class EPatrolOriginMode : uint8
{
	Initial UMETA(DisplayName = "Around Initial Location", ToolTip = "原点为出生时坐标"),
	Previous UMETA(DisplayName = "Around Previous Location", ToolTip = "原点为上次结束巡逻后的位置")
};

UENUM(BlueprintType)
enum class EPatrolRecoverMode : uint8
{
	Patrol UMETA(DisplayName = "Continue Patrolling", ToolTip = "继续巡逻"),
	Move UMETA(DisplayName = "Move By Flow Field", ToolTip = "按照寻路移动")
};


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FPatrol
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "启用巡逻行为"))
	bool bEnable = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "是否索敌"))
	bool bCanTrace = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "巡逻目标点最近距离"))
	float PatrolRadiusMin = 500;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "巡逻目标点最远距离"))
	float PatrolRadiusMax = 1000;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "每轮巡逻允许移动的最大时长"))
	float MaxMovingTime = 10.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "抵达目标点后的停留时长", DisplayName = "Wait Time"))
	float CoolDown = 2.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "距离低于该值时停止移动"))
	float AcceptanceRadius = 100;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "移动速度乘数"))
	float MoveSpeedMult = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "使用哪里作为原点"))
	EPatrolOriginMode OriginMode = EPatrolOriginMode::Initial;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "目标丢失后行为"))
	EPatrolRecoverMode OnLostTarget = EPatrolRecoverMode::Patrol;

	FVector Origin = FVector::ZeroVector;

};