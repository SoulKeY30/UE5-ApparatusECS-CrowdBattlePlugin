#pragma once

#include "CoreMinimal.h"
#include "Patrol.generated.h" 

UENUM(BlueprintType)
enum class EPatrolOrigin : uint8
{
	Initial UMETA(DisplayName = "Initial", ToolTip = "原点为出生时坐标"),
	Previous UMETA(DisplayName = "Previous", ToolTip = "原点为上次结束巡逻后的位置")
};


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FPatrol
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bEnable = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Range = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EPatrolOrigin Origin = EPatrolOrigin::Initial;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CoolDown = 3.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector2D Randomize = FVector2D(0.5,1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bCheckVisibility = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EObjectTypeQuery> TraceObjectType;

};