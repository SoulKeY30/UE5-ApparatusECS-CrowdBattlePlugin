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

UENUM(BlueprintType)
enum class ESpawnOrigin : uint8
{
    AtSelf UMETA(DisplayName = "AtSelf", Tooltip = "在自身位置"),
    AtTarget UMETA(DisplayName = "AtTarget", Tooltip = "在攻击目标位置")
};

UENUM(BlueprintType)
enum class EPlaySoundOrigin : uint8
{
    PlaySound2D UMETA(DisplayName = "PlaySound2D", Tooltip = "不使用空间音效"),
    PlaySound3D UMETA(DisplayName = "AtSelf", Tooltip = "使用空间音效"),
};

UENUM(BlueprintType)
enum class EPlaySoundOrigin_Attack : uint8
{
    PlaySound2D UMETA(DisplayName = "PlaySound2D", Tooltip = "不使用空间音效"),
    PlaySound3D_AtSelf UMETA(DisplayName = "AtSelf", Tooltip = "在自身位置"),
    PlaySound3D_AtTarget UMETA(DisplayName = "AtTarget", Tooltip = "在攻击目标位置")
};