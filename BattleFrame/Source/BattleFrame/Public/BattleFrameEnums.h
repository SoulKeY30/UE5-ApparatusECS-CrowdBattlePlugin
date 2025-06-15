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

UENUM(BlueprintType)
enum class EAttackState : uint8
{
	Aim UMETA(DisplayName = "Aim", ToolTip = "瞄准"),
	PreCast UMETA(DisplayName = "Begin", ToolTip = "前摇"),
	PostCast UMETA(DisplayName = "Hit", ToolTip = "后摇"),
	Cooling UMETA(DisplayName = "Cooling", ToolTip = "冷却")
};

UENUM(BlueprintType)
enum class EMoveState : uint8
{
	Dirty UMETA(DisplayName = "Dirty", ToolTip = "无效数据"),
	Sleeping UMETA(DisplayName = "Sleeping", ToolTip = "休眠中"),
	Patrolling UMETA(DisplayName = "Patrolling", ToolTip = "巡逻中"),
	PatrolWaiting UMETA(DisplayName = "PatrolWaiting", ToolTip = "在巡逻点位等待"),
	ChasingTarget UMETA(DisplayName = "ChasingTarget", ToolTip = "正在追逐目标"),
	ReachedTarget UMETA(DisplayName = "ReachedTarget", ToolTip = "已追到目标"),
	MovingToLocation UMETA(DisplayName = "MovingToLocation", ToolTip = "移动到位置"),
	ArrivedAtLocation UMETA(DisplayName = "ArrivedAtLocation", ToolTip = "已抵达目标位置")
};

// Event State
UENUM(BlueprintType)
enum class EAppearEventState : uint8
{
	Delay UMETA(DisplayName = "Delay", ToolTip = "延时"),
	Begin UMETA(DisplayName = "Begin", ToolTip = "开始"),
	End UMETA(DisplayName = "End", ToolTip = "结束")
};

UENUM(BlueprintType)
enum class ETraceEventState : uint8
{
	Begin UMETA(DisplayName = "Begin", ToolTip = "开始"),
	End_Reason_Succeed UMETA(DisplayName = "End_Reason_Succeed", ToolTip = "成功"),
	End_Reason_Fail UMETA(DisplayName = "End_Reason_Fail", ToolTip = "失败")
};

UENUM(BlueprintType)
enum class EMoveEventState : uint8
{
	Sleeping UMETA(DisplayName = "Sleeping", ToolTip = "休眠中"),
	Patrolling UMETA(DisplayName = "Patrolling", ToolTip = "巡逻中"),
	PatrolWaiting UMETA(DisplayName = "PatrolWaiting", ToolTip = "在巡逻点位等待"),
	ChasingTarget UMETA(DisplayName = "ChasingTarget", ToolTip = "正在追逐目标"),
	ReachedTarget UMETA(DisplayName = "ReachedTarget", ToolTip = "已追到目标"),
	MovingToLocation UMETA(DisplayName = "MovingToLocation", ToolTip = "移动到位置"),
	ArrivedAtLocation UMETA(DisplayName = "ArrivedAtLocation", ToolTip = "已抵达目标位置")
};

UENUM(BlueprintType)
enum class EAttackEventState : uint8
{
	Aiming UMETA(DisplayName = "Aiming", ToolTip = "瞄准"),
	Begin UMETA(DisplayName = "Begin", ToolTip = "开始"),
	Hit UMETA(DisplayName = "Hit", ToolTip = "击中"),
	Cooling UMETA(DisplayName = "Cooling", ToolTip = "冷却"),
	End_Reason_InvalidTarget UMETA(DisplayName = "End_Reason:InvalidTarget", ToolTip = "攻击目标失效"),
	End_Reason_Complete UMETA(DisplayName = "End_Reason:Complete", ToolTip = "完成")
};

UENUM(BlueprintType)
enum class EHitEventState : uint8
{
	Begin UMETA(DisplayName = "Begin", ToolTip = "开始")
};

UENUM(BlueprintType)
enum class EDeathEventState : uint8
{
	OutOfHealth UMETA(DisplayName = "OutOfHealth", ToolTip = "生命值归零"),
	OutOfLifeSpan UMETA(DisplayName = "OutOfLifeSpan", ToolTip = "寿命归零"),
	SuicideAttack UMETA(DisplayName = "SuicideAttack", ToolTip = "自杀攻击"),
	KillZ UMETA(DisplayName = "KillZ", ToolTip = "低于强制移除高度")
};