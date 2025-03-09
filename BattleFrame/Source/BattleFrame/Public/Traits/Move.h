#pragma once

#include "CoreMinimal.h"
#include "Move.generated.h"

UENUM(BlueprintType)
enum class ERotationMode : uint8
{
	RotationFollowVelocity UMETA(DisplayName = "RotationFollowVelocity", Tooltip = "旋转跟随速度方向"),
	VelocityFollowRotation UMETA(DisplayName = "VelocityFollowRotation", Tooltip = "速度跟随旋转方向")
};

/**
 * The movement speed factor.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FMove
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用移动属性"))
	bool bEnable = true;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "移动速度"))
	float Speed = 600.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "速度范围映射 (X: 最小距离时速度乘数, Y: 最小距离, Z:最大距离时速度乘数, W:最大距离 )"))
	FVector4 SpeedDistRangeMap = FVector4(0, 200, 1, 400);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "速度范围映射 (X: 最小转向角时速度乘数, Y: 最小转向角, Z:最大转向角时速度乘数, W:最大转向角 )"))
	FVector4 SpeedTurnRangeMap = FVector4(1, 90, 0.25, 180);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "加速度"))
	float Acceleration = 3.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "刹车加速度"))
	float BrakeDeceleration = 10.f;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "旋转模式 (RotationFollowVelocity: 旋转跟随速度, VelocityFollowRotation: 速度跟随旋转)"))
	ERotationMode RotationMode = ERotationMode::VelocityFollowRotation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "偏航旋转速度"))
	float RotationSpeed_Yaw = 3.f;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否可以飞行"))
	bool bCanFly = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "飞行高度范围 (X: 最小高度, Y: 最大高度)"))
	FVector2D FlyHeightRange = FVector2D(200.f, 400.f);


	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "重力值"))
	float Gravity = -4000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "死亡高度 (低于此高度时死亡)"))
	float KillZ = -2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "最大冲力"))
	float MaxImpulse = 5000.f;

	FMove() {};
};
