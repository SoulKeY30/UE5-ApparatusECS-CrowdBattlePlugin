/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#pragma once

// C++
#include <utility>

// Unreal
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StreamableManager.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "HAL/PlatformMisc.h"

// Apparatus
#include "Machine.h"
#include "Mechanism.h"

// BattleFrame
#include "BattleFrameFunctionLibraryRT.h"

#include "Traits/Debuff.h"
#include "Traits/DmgSphere.h"
#include "Traits/SubType.h"
#include "Traits/Animation.h"
#include "Traits/Move.h"
#include "Traits/Moving.h"
#include "Traits/Navigation.h"
#include "Traits/Trace.h"
#include "Traits/Death.h"
#include "Traits/Dying.h"
#include "Traits/DeathAnim.h"
#include "Traits/DeathDissolve.h"
#include "Traits/Appear.h"
#include "Traits/Appearing.h"
#include "Traits/AppearAnim.h"
#include "Traits/AppearDissolve.h"
#include "Traits/Rendering.h"
#include "Traits/RenderBatchData.h"
#include "Traits/Attack.h"
#include "Traits/Attacking.h"
#include "Traits/TemporalDamaging.h"
#include "Traits/Hit.h"
#include "Traits/HitGlow.h"
#include "Traits/Jiggle.h"
#include "Traits/Health.h"
#include "Traits/HealthBar.h"
#include "Traits/Damage.h"
#include "Traits/TextPopUp.h"
#include "Traits/Freezing.h"
#include "Traits/PoppingText.h"
#include "Traits/SpawningFx.h"
#include "Traits/Defence.h"
#include "Traits/Agent.h"
#include "Traits/SphereObstacle.h"
#include "Traits/Scaled.h"
#include "Traits/Directed.h"
#include "Traits/Located.h"
#include "Traits/Avoidance.h"
#include "Traits/Collider.h"
#include "Traits/Curves.h"
#include "Traits/Corpse.h"
#include "Traits/Statistics.h"
#include "Traits/BindFlowField.h"
#include "Traits/ValidSubjects.h"
#include "BattleFrameStructs.h"
#include "Traits/Sleep.h"
#include "Traits/Patrol.h"
#include "Traits/Sleeping.h"
#include "Traits/Patrolling.h"
#include "Traits/Burning.h"
#include "Traits/ActorSpawnConfig.h"
#include "Traits/SoundConfig.h"
#include "Traits/FxConfig.h"
#include "Traits/Activated.h"
#include "Traits/IsSubjective.h"
#include "DmgResultInterface.h"
#include "BattleFrameStructs.h"
#include "BattleFrameBattleControl.generated.h"

// Forward Declearation
class UNeighborGridComponent;

UCLASS()
class BATTLEFRAME_API ABattleFrameBattleControl : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = BattleFrame)
	int32 MaxThreadsAllowed = FMath::Clamp(FPlatformMisc::NumberOfWorkerThreadsToSpawn(), 1, 20);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = BattleFrame)
	int32 MinBatchSizeAllowed = 100;

	int32 ThreadsCount = 1;
	int32 BatchSize = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = BattleFrame)
	int32 NumSoundsPerFrame = 10;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = BattleFrame)
	float SoundVolume = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = BattleFrame)
	bool bGamePaused = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = BattleFrame)
	int32 AgentCount = 0;

	static ABattleFrameBattleControl* Instance;
	FStreamableManager StreamableManager;
	UWorld* CurrentWorld = nullptr;
	AMechanism* Mechanism = nullptr;
	TArray<UNeighborGridComponent*> NeighborGrids;
	TQueue<TSoftObjectPtr<USoundBase>, EQueueMode::Mpsc> SoundsToPlay;
	TQueue<float> VolumesToPlay;
	EFlagmarkBit ReloadFlowFieldFlag = EFlagmarkBit::R;
	TQueue<FDmgResult, EQueueMode::Mpsc> DamageResultQueue;
	TSet<int32> ExistingRenderers;

private:

	// all filters we gonna use
	bool bIsFilterReady = false;
	FFilter AgentCountFilter;
	FFilter AgentAgeFilter;
	FFilter AgentAppeaFilter;
	FFilter AgentAppearAnimFilter;
	FFilter AgentAppearDissolveFilter;
	FFilter AgentTraceFilter;
	FFilter AgentAttackFilter;
	FFilter AgentAttackingFilter;
	FFilter AgentHitGlowFilter;
	FFilter AgentJiggleFilter;
	FFilter AgentBurningFilter;
	FFilter AgentFrozenFilter;
	FFilter DecideDamageFilter;
	FFilter AgentHealthBarFilter;
	FFilter AgentDeathFilter;
	FFilter AgentDeathDissolveFilter;
	FFilter AgentDeathAnimFilter;
	FFilter SpeedLimitOverrideFilter;
	FFilter AgentPatrolFilter;
	FFilter AgentMoveFilter;
	FFilter IdleToMoveAnimFilter;
	FFilter AgentStateMachineFilter;
	FFilter RenderBatchFilter;
	FFilter AgentRenderFilter;
	FFilter TextRenderFilter;
	FFilter SpawnActorsFilter;
	FFilter SpawnFxFilter;
	FFilter PlaySoundFilter;


public:

	ABattleFrameBattleControl()
	{
		PrimaryActorTick.bCanEverTick = true;
	}

	void BeginPlay() override;

	void EndPlay(const EEndPlayReason::Type EndPlayReason) override
	{
		if (Instance == this)
		{
			Instance = nullptr;
		}

		Super::EndPlay(EndPlayReason);
	}

	void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static ABattleFrameBattleControl* GetInstance()
	{
		return Instance;
	}

	void ApplyDamageToSubjects(const FSubjectArray& Subjects, const FSubjectArray& IgnoreSubjects, const FSubjectHandle DmgInstigator, const FVector& HitFromLocation, const FDmgSphere& DmgSphere, const FDebuff& Debuff, TArray<FDmgResult>& DamageResults);

	static FVector FindNewPatrolGoalLocation(const FPatrol& Patrol, const FCollider& Collider, const FTrace& Trace, const FLocated& Located, int32 MaxAttempts);

	void DefineFilters();

	// 计算实际伤害，并返回一个pair，第一个元素是是否暴击，第二个元素是实际伤害
	FORCEINLINE std::pair<bool, float> ProcessCritDamage(float BaseDamage, float damageMult, float Probability)
	{
		//TRACE_CPUPROFILER_EVENT_SCOPE_STR("ProcessCrit");
		float ActualDamage = BaseDamage;
		bool IsCritical = false;  // 是否暴击

		// 生成一个[0, 1]范围内的随机数
		float CritChance = FMath::FRand();

		// 判断是否触发暴击
		if (CritChance < Probability)
		{
			ActualDamage *= damageMult;  // 应用暴击倍数
			IsCritical = true;  // 设置暴击标志
		}

		return { IsCritical, ActualDamage };  // 返回pair
	}

	FORCEINLINE void CopyAnimData(FAnimation& Animation)
	{
		Animation.AnimLerp = 0;
		Animation.AnimIndex0 = Animation.AnimIndex1;
		Animation.AnimCurrentTime0 = Animation.AnimCurrentTime1;
		Animation.AnimOffsetTime0 = Animation.AnimOffsetTime1;
		Animation.AnimPauseTime0 = Animation.AnimPauseTime1;
		Animation.AnimPlayRate0 = Animation.AnimPlayRate1;
	}

	FORCEINLINE static FTransform LocalOffsetToWorld(FQuat WorldRotation, FVector WorldLocation, FTransform LocalTransform)
	{
		// 计算世界空间的位置偏移
		FVector WorldLocationOffset = WorldRotation.RotateVector(LocalTransform.GetLocation());
		FVector FinalLocation = WorldLocation + WorldLocationOffset;

		// 组合旋转（世界朝向 + 本地偏移旋转）
		FQuat FinalRotation = WorldRotation * LocalTransform.GetRotation();

		return FTransform(FinalRotation, FinalLocation, LocalTransform.GetScale3D());
	}

	FORCEINLINE void QueueActor(FActorSpawnConfig Config)
	{
		if (!Config.bEnable) return;
		Config.Transform = ABattleFrameBattleControl::LocalOffsetToWorld(Config.SubjectTrans.GetRotation(), Config.SubjectTrans.GetLocation(), Config.Transform);
		Mechanism->SpawnSubjectDeferred(Config);
	}

	FORCEINLINE void QueueSound(FSoundConfig Config)
	{
		if (!Config.bEnable) return;
		Config.Transform = ABattleFrameBattleControl::LocalOffsetToWorld(Config.SubjectTrans.GetRotation(), Config.SubjectTrans.GetLocation(), Config.Transform);
		Mechanism->SpawnSubjectDeferred(Config);
	}

	FORCEINLINE void QueueFx(FFxConfig Config)
	{
		if (!Config.bEnable) return;
		Config.Transform = ABattleFrameBattleControl::LocalOffsetToWorld(Config.SubjectTrans.GetRotation(), Config.SubjectTrans.GetLocation(), Config.Transform);
		Mechanism->SpawnSubjectDeferred(Config);
	}

	FORCEINLINE void QueueText(FSubjectHandle Subject, float Value, float Style, float Scale, float Radius, FVector Location)
	{
		//TRACE_CPUPROFILER_EVENT_SCOPE_STR("QueueText");

		if (Subject.HasTrait<FPoppingText>())
		{
			auto& PoppingText = Subject.GetTraitRef<FPoppingText, EParadigm::Unsafe>();

			PoppingText.Lock();
			PoppingText.TextLocationArray.Add(Location);
			PoppingText.Text_Value_Style_Scale_Offset_Array.Add(FVector4(Value, Style, Scale, Radius));
			//UE_LOG(LogTemp, Warning, TEXT("OldTrait"));
			PoppingText.Unlock();
		}
		else
		{
			FPoppingText NewPoppingText;
			NewPoppingText.TextLocationArray.Add(Location);
			NewPoppingText.Text_Value_Style_Scale_Offset_Array.Add(FVector4(Value, Style, Scale, Radius));
			//UE_LOG(LogTemp, Warning, TEXT("NewTrait"));

			Subject.SetTraitDeferred(NewPoppingText);
		}
	}


};