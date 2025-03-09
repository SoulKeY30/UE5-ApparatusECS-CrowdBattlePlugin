#pragma once
 
#include "CoreMinimal.h"
#include "Math/Vector.h"
#include "Shoot.generated.h"


/**
 * The attack command for the player.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FShoot
{
	GENERATED_BODY()
 
  public:

	/* 发射模式*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Mode = 0;
	
	/* 射击频率*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Frequency = 3.f; 

	/* 分裂射击分叉数量*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int AdditionalStreamCount = 0; 

	/* 分裂射击每个分叉的夹角*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AdditionalStreamDistance = 700.f; 

	/* 搜索半径，射程*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Range = 6500.0f; 

	/* 预判迭代次数*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int PredictPrecision = 3; 

	/* 抛射仰角*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxPitchAngle = 45.0f; 

	/* 抛射仰角*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MinPitchAngle = 15.0f; 

	/* 射击角度误差*/
	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AngleRandom = 0.f; 

	/* 射击初始位置误差*/
	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector2D FromLocationRandom = FVector2D(0.f, 0.f);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Gravity = 8000.f; 

	/* 在最近的这么多怪物中选择一个来攻击*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 pickOneAmongst = 3;

	/* 如果这段时间后目标还存活，重新进行一次索敌*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float retargettingTime = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float targettingAgainTime = 0.5f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float traceCompensationMult = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Priority = 0;

};
