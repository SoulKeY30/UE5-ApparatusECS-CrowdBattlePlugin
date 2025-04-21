#pragma once

#include "CoreMinimal.h"
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
	Patrol UMETA(DisplayName = "Continue Patrolling", ToolTip = ""),
	Move UMETA(DisplayName = "Move By Flow Field", ToolTip = "")
};


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FPatrol
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bEnable = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PatrolRadiusMin = 500;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PatrolRadiusMax = 1000;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "速度范围映射 (X: 距移动目标点最小距离, Y: 最小距离时速度乘数, Z:距移动目标点最大距离, W:最大距离时速度乘数 )"))
	FVector4 PatrolSpeedRangeMap = FVector4(100, 0, 200, 1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "索敌视野半径"))
	float TraceRadius = 1000.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = 0, UIMax = 360, Tooltip = "索敌视野角度"))
	float TraceAngle = 360.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "检测目标是不是在障碍物后面"))
	bool bCheckVisibility = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxMovingTime = 5.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CoolDown = 2.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EPatrolOriginMode OriginMode = EPatrolOriginMode::Initial;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EPatrolRecoverMode OnLostTarget = EPatrolRecoverMode::Patrol;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Origin = FVector::ZeroVector;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Goal = FVector::ZeroVector;

};