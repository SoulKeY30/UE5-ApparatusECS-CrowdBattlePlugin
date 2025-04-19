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
	float MinRadius = 500;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxRadius = 1000;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxDuration = 5.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CoolDown = 2.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EPatrolOriginMode OriginMode = EPatrolOriginMode::Initial;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EPatrolRecoverMode OnLostTarget = EPatrolRecoverMode::Patrol;

	FVector Origin = FVector::ZeroVector;

	FVector Goal = FVector::ZeroVector;

};