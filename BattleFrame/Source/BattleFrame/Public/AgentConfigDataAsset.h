/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#pragma once

#include "Engine/DataAsset.h"
#include "SubjectRecord.h"

// 移动相关 Traits
#include "Traits/Move.h"
#include "Traits/Navigation.h"
#include "Traits/Trace.h"
#include "Traits/Death.h"
#include "Traits/Appear.h"
#include "Traits/Attack.h"
#include "Traits/Hit.h"
#include "Traits/Health.h"
#include "Traits/HealthBar.h"
#include "Traits/Damage.h"
#include "Traits/TextPopUp.h"
#include "Traits/Debuff.h"
#include "Traits/Defence.h"
#include "Traits/Agent.h"
#include "Traits/Scaled.h"
#include "Traits/Collider.h"
#include "Traits/Avoidance.h"
#include "Traits/Animation.h"
#include "Traits/Sleep.h"
#include "Traits/Patrol.h"
#include "Traits/Perception.h"
#include "Traits/Curves.h"
#include "Traits/Statistics.h"
#include "Traits/CustomData.h"

#include "AgentConfigDataAsset.generated.h"



UCLASS(Blueprintable, BlueprintType)
class BATTLEFRAME_API UAgentConfigDataAsset : public UPrimaryDataAsset
{
public:

	GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "主类型"))
    FAgent Agent;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "子类型"))
    FSubType SubType;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "缩放"))
    FScaled Scale;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "碰撞体"))
    FCollider Collider;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "血量"))
    FHealth Health;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "动画"))
    FAnimation Animation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "曲线"))
    FCurves Curves;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "出生"))
    FAppear Appear;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "休眠"))
    FSleep Sleep;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "巡逻"))
    FPatrol Patrol;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "索敌"))
    FTrace Trace;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "移动"))
    FMove Move;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "导航"))
    FNavigation Navigation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "避障/碰撞"))
    FAvoidance Avoidance;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "攻击"))
    FAttack Attack;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "伤害值"))
    FDamage Damage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "减益"))
    FDebuff Debuff;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "受击"))
    FHit Hit;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "血条"))
    FHealthBar HealthBar;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "数字"))
    FTextPopUp TextPop;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "抗性"))
    FDefence Defence;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "死亡"))
    FDeath Death;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "自定义数据"))
    FCustomData CustomData;

    //UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "数据统计"))
    //FStatistics Statistics;

    UAgentConfigDataAsset() {};

};
