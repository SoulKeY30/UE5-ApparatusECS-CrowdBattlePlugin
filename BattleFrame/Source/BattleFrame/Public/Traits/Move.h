#pragma once

#include "CoreMinimal.h"
#include "Move.generated.h"


UENUM(BlueprintType)
enum class EOrientMode : uint8
{
	ToPath UMETA(DisplayName = "ToPath"),
	ToMovement UMETA(DisplayName = "ToMovement"),
	ToMovementForwardAndBackward UMETA(DisplayName = "ToMovementForwardAndBackward")
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FMove
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用移动属性"))
	bool bEnable = true;

	//---------------Yaw Movement-----------------//

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "转向角速度"))
	float TurnSpeed = 1000;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "转向角加速度"))
	float TurnAcceleration = 1000;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "转向角加速度"))
	EOrientMode TurnMode = EOrientMode::ToMovementForwardAndBackward;


	//---------------XY Movement-----------------//

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "移动速度"))
	float MoveSpeed = 600.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "移动加速度"))
	float MoveAcceleration = 3000.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "刹车减速度"))
	float MoveDeceleration = 3000.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "速度映射 (X: 与移动方向的夹角, Y: 对应的速度乘数, Z:与移动方向的夹角, W:对应的速度乘数)"))
	FVector4 MoveSpeedRangeMapByAngle = FVector4(0, 1, 180, 1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "速度映射 (X: 与目标点的距离, Y: 对应的速度乘数, Z:与目标点的距离, W:对应的速度乘数)"))
	FVector4 MoveSpeedRangeMapByDist = FVector4(100, 1, 1000, 1);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0", Tooltip = "每次地面弹跳移动速度衰减"))
	FVector2D MoveBounceVelocityDecay = FVector2D(0.5f, 0.f);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "与移动目标点距离低于该值时停止移动"))
	float AcceptanceRadius = 100.f;


	//---------------Z Movement-----------------//

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否可以飞行"))
	bool bCanFly = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "飞行高度 (X: 最小高度, Y: 最大高度)"))
	FVector2D FlyHeightRange = FVector2D(200.f, 400.f);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "重力"))
	float Gravity = -2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "强制死亡高度 (低于此高度时强制移除)"))
	float KillZ = -4000.f;

	//---------------Draw Debug-----------------//

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "绘制Debug图形"))
	bool bDrawDebugShape = false;

};
