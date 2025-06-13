#pragma once

#include "CoreMinimal.h"
#include "Chase.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FChase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用索敌功能(WIP)"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "与攻击目标距离低于该值时停止移动"))
	float AcceptanceRadius = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "移动速度乘数(WIP)"))
	float MoveSpeedMult = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "Chase专用索敌扇形视野的尺寸"))
	FSectorTraceParamsSpecific SectorParams = FSectorTraceParamsSpecific();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "绘制Debug图形"))
	bool bDrawDebugShape = false;

};
