#pragma once

#include "CoreMinimal.h"
#include "BattleFrameStructs.h"
#include "Sleep.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSleep
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bEnable = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bWakeOnHit = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "Sleep专用索敌扇形视野的尺寸"))
	FSectorTraceParamsSpecific SectorParams = FSectorTraceParamsSpecific();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "绘制Debug图形"))
	bool bDrawDebugShape = false;

};