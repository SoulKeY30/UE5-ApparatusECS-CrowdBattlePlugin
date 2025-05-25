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

UENUM(BlueprintType)
enum class EMoveStyle : uint8
{
    Creature UMETA(DisplayName = "Creature", ToolTip = "生物先向任意方向移动，然后转身朝向移动的方向"),
    Vehicle UMETA(DisplayName = "Vehicle", ToolTip = "车辆先转向，然后按当前朝向前进或后退")
};