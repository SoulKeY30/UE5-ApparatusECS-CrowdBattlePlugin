#pragma once

#include "CoreMinimal.h"
#include "Sleep.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSleep
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bEnable = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "索敌视野半径"))
	float TraceRadius = 1000.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = 0, UIMax = 360, Tooltip = "索敌视野角度"))
	float TraceAngle = 360.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "检测目标是不是在障碍物后面"))
	bool bCheckVisibility = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bWakeOnHit = true;

};