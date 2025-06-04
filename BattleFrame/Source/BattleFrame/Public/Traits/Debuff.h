#pragma once

#include "CoreMinimal.h"
#include "Debuff.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FLaunchParams
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否可以将目标击退"))
	bool bCanLaunch = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "击退的速度,分为XY与Z"))
	FVector2D LaunchSpeed = FVector2D(2000, 0);

};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTemporalDmgParams
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否可以造成延时伤害"))
	bool bDealTemporalDmg = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "总延时伤害"))
	float TemporalDmg = 60.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "分段数量"))
	int32 TemporalDmgSegment = 6;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "伤害间隔"))
	float TemporalDmgInterval = 0.5f;

};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSlowParams
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否可以使目标减速"))
	bool bCanSlow = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "减速的持续时间（秒）"))
	float SlowTime = 4.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "减速的强度"))
	float SlowStrength = 1.f;

};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDebuff
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用减益"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "击退"))
	FLaunchParams LaunchParams = FLaunchParams();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "延时伤害"))
	FTemporalDmgParams TemporalDmgParams = FTemporalDmgParams();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "减速"))
	FSlowParams SlowParams = FSlowParams();
};