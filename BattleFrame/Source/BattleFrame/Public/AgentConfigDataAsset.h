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
#include "Traits/SpawnActor.h"
#include "Traits/Hit.h"
#include "Traits/Health.h"
#include "Traits/HealthBar.h"
#include "Traits/Damage.h"
#include "Traits/TextPopUp.h"
#include "Traits/Debuff.h"
#include "Traits/Sound.h"
#include "Traits/FX.h"
#include "Traits/Defence.h"
#include "Traits/Agent.h"
#include "Traits/Scaled.h"
#include "Traits/Collider.h"
#include "Traits/Avoidance.h"
#include "Traits/Chase.h"
#include "Traits/Escape.h"
#include "Traits/Animation.h"
#include "Traits/Sleep.h"
#include "Traits/Patrol.h"
#include "Traits/Perception.h"
#include "Traits/Curves.h"
#include "Traits/Statistics.h"

#include "AgentConfigDataAsset.generated.h"



UCLASS(Blueprintable, BlueprintType)
class BATTLEFRAME_API UAgentConfigDataAsset : public UPrimaryDataAsset
{
public:

	GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "主类型标签"))
    FAgent Agent;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "子类型属性"))
    FSubType SubType;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "碰撞体属性"))
    FCollider Collider;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "缩放属性"))
    FScaled Scale;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "生命值属性"))
    FHealth Health;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "伤害属性"))
    FDamage Damage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "攻击造成减益效果"))
    FDebuff Debuff;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "防御属性"))
    FDefence Defence;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "移动属性"))
    FMove Move;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "导航属性"))
    FNavigation Navigation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "避障属性"))
    FAvoidance Avoidance;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "出生属性"))
    FAppear Appear;

    //UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "睡眠属性"))
    //FSleep Sleep;

    //UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "巡逻属性"))
    //FPatrol Patrol;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "索敌属性"))
    FTrace Trace;

    //UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "追逐属性"))
    //FChase Chase;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "攻击属性"))
    FAttack Attack;

    //UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "逃跑属性"))
    //FEscape Escape;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "受击属性"))
    FHit Hit;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "死亡属性"))
    FDeath Death;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "动画属性"))
    FAnimation Animation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "生命条属性"))
    FHealthBar HealthBar;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "文本弹出属性"))
    FTextPopUp TextPop;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "特效属性"))
    FFX FX;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "音效属性"))
    FSound Sound;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "生成Actor属性"))
    FSpawnActor SpawnActor;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "曲线属性"))
    FCurves Curves;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "数据统计"))
    FStatistics Statistics;

    UAgentConfigDataAsset() {}

};
