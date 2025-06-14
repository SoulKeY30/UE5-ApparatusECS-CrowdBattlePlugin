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

UENUM(BlueprintType)
enum class EInitialDirection : uint8
{
	FacePlayer UMETA(
		DisplayName = "FacingPlayer0",
		Tooltip = "生成时面朝玩家0的方向"
	),
		FaceForward UMETA(
			DisplayName = "SpawnerForwardVector",
			Tooltip = "使用生成器的前向向量作为朝向"
		),
		CustomDirection UMETA(
			DisplayName = "CustomDirection",
			Tooltip = "自定义朝向"
		)
};

// Event State
UENUM(BlueprintType)
enum class EAppearState : uint8
{
	Delay UMETA(DisplayName = "Delay", ToolTip = "延时"),
	Begin UMETA(DisplayName = "Begin", ToolTip = "开始"),
	End UMETA(DisplayName = "End", ToolTip = "结束")
};

UENUM(BlueprintType)
enum class ESleepState : uint8
{
	End UMETA(DisplayName = "End", ToolTip = "结束(惊醒)")
};

UENUM(BlueprintType)
enum class EPatrolState : uint8
{
	Begin UMETA(DisplayName = "Begin", ToolTip = "开始"),
	Wait UMETA(DisplayName = "Wait", ToolTip = "在目的地等待"),
	End UMETA(DisplayName = "End", ToolTip = "结束")
};

UENUM(BlueprintType)
enum class ETraceState : uint8
{
	Succeed UMETA(DisplayName = "Succeed", ToolTip = "成功"),
	Fail UMETA(DisplayName = "Fail", ToolTip = "失败")
};

UENUM(BlueprintType)
enum class EMoveState : uint8
{
	Idle UMETA(DisplayName = "Idle", ToolTip = ""),
	Moving UMETA(DisplayName = "Moving", ToolTip = ""),
	Chasing UMETA(DisplayName = "Chasing", ToolTip = ""),
	Falling UMETA(DisplayName = "Falling", ToolTip = ""),
	Launched UMETA(DisplayName = "Launched", ToolTip = ""),
	Pushed UMETA(DisplayName = "Pushed", ToolTip = "")
};

UENUM(BlueprintType)
enum class EAttackState : uint8
{
	Aim UMETA(DisplayName = "Aim", ToolTip = "瞄准(开始)"),
	PreCast UMETA(DisplayName = "PreCast", ToolTip = "前摇"),
	Hit UMETA(DisplayName = "Hit", ToolTip = "击中"),
	PostCast UMETA(DisplayName = "PostCast", ToolTip = "后摇"),
	Cooling UMETA(DisplayName = "Cooling", ToolTip = "冷却"),
	End UMETA(DisplayName = "End", ToolTip = "结束")
};

UENUM(BlueprintType)
enum class EHitState : uint8
{
	Begin UMETA(DisplayName = "Begin", ToolTip = "")
};

UENUM(BlueprintType)
enum class EDeathState : uint8
{
	Begin UMETA(DisplayName = "Begin", ToolTip = "开始"),
	End UMETA(DisplayName = "End", ToolTip = "结束")
};