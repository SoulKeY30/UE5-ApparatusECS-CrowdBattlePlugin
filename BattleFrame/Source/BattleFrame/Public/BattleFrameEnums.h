#pragma once

#include "CoreMinimal.h"
#include "BattleFrameEnums.generated.h"

UENUM(BlueprintType)
enum class ESortMode : uint8
{
    None UMETA(DisplayName = "None", ToolTip = "不排序"),
    NearToFar UMETA(DisplayName = "Near to Far", ToolTip = "从近到远"),
    FarToNear UMETA(DisplayName = "Far to Near", ToolTip = "从远到近")
};