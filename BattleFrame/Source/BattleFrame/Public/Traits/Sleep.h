#pragma once

#include "CoreMinimal.h"
#include "BattleFrameStructs.h"
#include "Sleep.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSleep
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "启用休眠行为"))
	bool bEnable = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "是否索敌"))
	bool bCanTrace = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "击中后唤醒"))
	bool bWakeOnHit = true;

};