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
#include "Traits/TemporalDamager.h"
#include "Traits/Hit.h"
#include "Traits/HitGlow.h"
#include "Traits/Jiggle.h"
#include "Traits/Health.h"
#include "Traits/HealthBar.h"
#include "Traits/Damage.h"
#include "Traits/TextPopUp.h"
#include "Traits/Slowing.h"
#include "Traits/Slower.h"
#include "Traits/PoppingText.h"
#include "Traits/SpawningFx.h"
#include "Traits/Defence.h"
#include "Traits/Agent.h"
#include "Traits/SphereObstacle.h"
#include "Traits/Scaled.h"
#include "Traits/Directed.h"
#include "Traits/Located.h"
#include "Traits/Avoidance.h"
#include "Traits/Avoiding.h"
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
#include "Traits/TemporalDamaging.h"
#include "Traits/ActorSpawnConfig.h"
#include "Traits/SoundConfig.h"
#include "Traits/FxConfig.h"
#include "Traits/Activated.h"
#include "Traits/IsSubjective.h"
#include "Traits/TextPopConfig.h"
#include "Traits/OwnerSubject.h"
#include "DmgResultInterface.h"
#include "Math/UnrealMathUtility.h"
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
	bool bGamePaused = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = BattleFrame)
	int32 AgentCount = 0;

	static ABattleFrameBattleControl* Instance;
	FStreamableManager StreamableManager;
	UWorld* CurrentWorld = nullptr;
	AMechanism* Mechanism = nullptr;
	TArray<UNeighborGridComponent*> NeighborGrids;
	EFlagmarkBit ReloadFlowFieldFlag = EFlagmarkBit::F;
	EFlagmarkBit HasPoppingTextFlag = EFlagmarkBit::T;
	//EFlagmarkBit NeedSettleDmgFlag = EFlagmarkBit::D;
	TQueue<FDmgResult, EQueueMode::Mpsc> DamageResultQueue;
	TSet<int32> ExistingRenderers;

	TQueue<FDebugPointConfig, EQueueMode::Mpsc> DebugPointQueue;
	TQueue<FDebugLineConfig, EQueueMode::Mpsc> DebugLineQueue;
	TQueue<FDebugSphereConfig, EQueueMode::Mpsc> DebugSphereQueue;
	TQueue<FDebugCapsuleConfig, EQueueMode::Mpsc> DebugCapsuleQueue;
	TQueue<FDebugSectorConfig, EQueueMode::Mpsc> DebugSectorQueue;
	TQueue<FDebugCircleConfig, EQueueMode::Mpsc> DebugCircleQueue;

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
	FFilter TemporalDamagerFilter;
	FFilter SlowerFilter;
	FFilter DecideHealthFilter;
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
	FFilter SubjectFilterBase;


public:

	ABattleFrameBattleControl()
	{
		PrimaryActorTick.bCanEverTick = true;
	}

	void BeginPlay() override;

	void Tick(float DeltaTime) override;

	void EndPlay(const EEndPlayReason::Type EndPlayReason) override
	{
		if (Instance == this)
		{
			Instance = nullptr;
		}

		Super::EndPlay(EndPlayReason);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static ABattleFrameBattleControl* GetInstance()
	{
		return Instance;
	}

	void DefineFilters();

	void ApplyDamageToSubjects(const FSubjectArray& Subjects, const FSubjectArray& IgnoreSubjects, const FSubjectHandle DmgInstigator, const FVector& HitFromLocation, const FDamage& FDamage, const FDebuff& Debuff, TArray<FDmgResult>& DamageResults);

	void ApplyDamageToSubjects(const FSubjectArray& Subjects, const FSubjectArray& IgnoreSubjects, const FSubjectHandle DmgInstigator, const FVector& HitFromLocation, const FDmgSphere& DmgSphere, const FDebuff& Debuff, TArray<FDmgResult>& DamageResults);

	void ApplyDamageToSubjectsDeferred(const FSubjectArray& Subjects, const FSubjectArray& IgnoreSubjects, const FSubjectHandle DmgInstigator, const FVector& HitFromLocation, const FDamage& FDamage, const FDebuff& Debuff, TArray<FDmgResult>& DamageResults);

	void ApplyDamageToSubjectsDeferred(const FSubjectArray& Subjects, const FSubjectArray& IgnoreSubjects, const FSubjectHandle DmgInstigator, const FVector& HitFromLocation, const FDmgSphere& DmgSphere, const FDebuff& Debuff, TArray<FDmgResult>& DamageResults);

	static FVector FindNewPatrolGoalLocation(const FPatrol& Patrol, const FCollider& Collider, const FTrace& Trace, const FLocated& Located, const FScaled& Scaled, int32 MaxAttempts);

	void DrawDebugSector(UWorld* World, const FVector& Center, const FVector& Direction, float Radius, float AngleDegrees, float Height, const FColor& Color, bool bPersistentLines, float LifeTime, uint8 DepthPriority, float Thickness);

	//---------------------------------------------RVO2------------------------------------------------------------------

	static void ComputeAvoidingVelocity(FAvoidance& Avoidance, FAvoiding& Avoiding, const TArray<FGridData>& SubjectNeighbors, const TArray<FGridData>& ObstacleNeighbors, float TimeStep);

	static bool LinearProgram1(const std::vector<RVO::Line>& lines, size_t lineNo, float radius, const RVO::Vector2& optVelocity, bool directionOpt, RVO::Vector2& result);

	static size_t LinearProgram2(const std::vector<RVO::Line>& lines, float radius, const RVO::Vector2& optVelocity, bool directionOpt, RVO::Vector2& result);

	static void LinearProgram3(const std::vector<RVO::Line>& lines, size_t numObstLines, size_t beginLine, float radius, RVO::Vector2& result);


	//---------------------------------------------Helpers------------------------------------------------------------------

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

	FORCEINLINE void QueueText(FTextPopConfig Config) const
	{
		//TRACE_CPUPROFILER_EVENT_SCOPE_STR("QueueText");

		if (Config.Owner.HasTrait<FPoppingText>())
		{
			auto& PoppingText = Config.Owner.GetTraitRef<FPoppingText, EParadigm::Unsafe>();

			PoppingText.Lock();
			PoppingText.TextLocationArray.Add(Config.Location);
			PoppingText.Text_Value_Style_Scale_Offset_Array.Add(FVector4(Config.Value, Config.Style, Config.Scale, Config.Radius));
			//UE_LOG(LogTemp, Warning, TEXT("OldTrait"));
			PoppingText.Unlock();

			Config.Owner.SetFlag(HasPoppingTextFlag,true);
		}
	}
};
