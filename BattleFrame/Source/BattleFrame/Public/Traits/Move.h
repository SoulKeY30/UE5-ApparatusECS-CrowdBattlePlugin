#pragma once

#include "CoreMinimal.h"
#include "Move.generated.h"


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

	//---------------Yaw Movement-----------------//

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "转向速度"))
	float TurnSpeed = 3.f;


	//---------------XY Movement-----------------//

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "移动速度"))
	float MoveSpeed = 600.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "速度范围映射 (X: 与移动方向的夹角, Y: 对应的速度乘数, Z:与移动方向的夹角, W:对应的速度乘数)"))
	FVector4 SpeedRangeMapByAngle = FVector4(0, 1, 180, 1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "速度范围映射 (X: 与目标点的距离, Y: 对应的速度乘数, Z:与目标点的距离, W:对应的速度乘数)"))
	FVector4 SpeedRangeMapByDist = FVector4(100, 1, 1000, 1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "距离低于该值时停止移动"))
	float AcceptanceRadius = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "加速度"))
	float Acceleration = 3000.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "地面摩擦减速度"))
	float Deceleration_Ground = 3000.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "空气阻力减速度"))
	float Deceleration_Falling = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "每次地面弹跳速度衰减"))
	FVector2D BounceVelocityDecay = FVector2D(0.5f, 0.f);

	//---------------Z Movement-----------------//

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否飞行"))
	bool bCanFly = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "出生时飞行高度 (X: 最小高度, Y: 最大高度)"))
	FVector2D FlyHeightRange = FVector2D(200.f, 400.f);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "重力"))
	float Gravity = -1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "强制死亡高度 (低于此高度时强制移除)"))
	float KillZ = -4000.f;

	FVector Goal = FVector(0, 0, 0);

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "速度下限"))
	//float MinMoveSpeed = 100.f;

};
