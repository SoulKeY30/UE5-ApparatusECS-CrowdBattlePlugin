/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#include "BattleFrameGameMode.h"
#include "Kismet/GameplayStatics.h"

// Niagara 插件
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

// Apparatus 插件
#include "SubjectiveActorComponent.h"

// FlowFieldCanvas 插件
#include "FlowField.h"

// BattleFrame 插件
#include "NeighborGridActor.h"
#include "NeighborGridComponent.h"
#include "BattleFrameFunctionLibraryRT.h"

// 移动相关 Traits
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
#include "Traits/SpawningActor.h"
#include "Traits/Rendering.h"
#include "Traits/RenderBatchData.h"
#include "Traits/Attack.h"
#include "Traits/Attacking.h"
#include "Traits/TemporalDamaging.h"
#include "Traits/Tracing.h"
#include "Traits/SpawnActor.h"
#include "Traits/Hit.h"
#include "Traits/BeingHit.h"
#include "Traits/HitGlow.h"
#include "Traits/SqueezeSquash.h"
#include "Traits/Health.h"
#include "Traits/HealthBar.h"
#include "Traits/Damage.h"
#include "Traits/TextPopUp.h"
#include "Traits/Freezing.h"
#include "Traits/PoppingText.h"
#include "Traits/Sound.h"
#include "Traits/FX.h"
#include "Traits/SpawningFx.h"
#include "Traits/Defence.h"
#include "Traits/Agent.h"
#include "Traits/RoadBlock.h"
#include "Traits/Scaled.h"
#include "Traits/Directed.h"
#include "Traits/Located.h"
#include "Traits/Avoidance.h"
#include "Traits/Collider.h"
#include "Traits/Curves.h"
#include "Traits/Static.h"
#include "Traits/Statistics.h"


ABattleFrameGameMode* ABattleFrameGameMode::Instance = nullptr;

void ABattleFrameGameMode::BeginPlay()
{
	Super::BeginPlay();

	Instance = this;
	CurrentWorld = GetWorld();
	Mechanism = GetMechanism();
	if (ANeighborGridActor::GetInstance()) { NeighborGrid = ANeighborGridActor::GetInstance()->GetComponent(); }
	if (bIsGameOver || !CurrentWorld || !Mechanism || !NeighborGrid) return;
}

void ABattleFrameGameMode::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("DMTDGameModeTick");

	Super::Tick(DeltaTime);

	if (bIsGameOver || !CurrentWorld || !Mechanism || !NeighborGrid) return;

	//----------------------出生逻辑-------------------------

	// 统计Agent数量
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("CountAgent");

		FFilter Filter = FFilter::Make<FAgent>();
		auto Chain = Mechanism->Enchain(Filter);
		AgentCount = Chain->IterableNum();
		Chain->Reset(true);
	}
	#pragma endregion

	// 统计游戏时长
	#pragma region
	{
		FFilter Filter = FFilter::Make<FStatistics>();
		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FStatistics& Stats)
			{
				if (Stats.bEnable)
				{
					Stats.totalTime += DeltaTime;
				}

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	//----------------------出生逻辑-------------------------

	// 出生总
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentAppearMain");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FLocated, FDirected, FAppear, FAppearing, FSpawnActor, FFX, FSound>();
		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FLocated& Located,
				FDirected& Directed,
				FAppear& Appear,
				FAppearing& Appearing,
				FAnimation& Animation,
				FSpawnActor& SpawnActor,
				FFX& FX,
				FSound& Sound)
				{
					// Initial execute
					if (Appearing.time == 0)
					{
						// Spawn Decal Actor
						if (Appear.bCanSpawnDecal && SpawnActor.AppearDecalClass)
						{
							FSpawningActor AppearSpawning;
							AppearSpawning.SpawnClass = SpawnActor.AppearDecalClass;
							AppearSpawning.Trans = FTransform(FRotator::ZeroRotator, FLocated{ Located }, FVector::OneVector);

							FSubjectRecord Record;
							Record.SetTrait(AppearSpawning);

							Mechanism->SpawnSubjectDeferred(Record);
						}
					}
					else if (Appearing.time <= Appear.Delay)
					{
						// Wait
					}
					else if (Appearing.time > Appear.Delay && Appearing.time <= (Appear.Duration + Appear.Delay))
					{
						if (!Appear.bAppearStarted)
						{
							Appear.bAppearStarted = true;

							// Dissolve In
							if (Appear.bCanDissolveIn)
							{
								Subject.SetTraitDeferred(FAppearDissolve{});
							}

							// Animation
							if (Appear.bCanPlayAnim)
							{
								Subject.SetTraitDeferred(FAppearAnim{});
							}

							// Fx
							if (Appear.bCanSpawnFx && FX.AppearFx.SubType != ESubType::None)
							{
								FRotator CombinedRotator = (FQuat(FX.AppearFx.Transform.GetRotation()) * FQuat(Directed.Direction.Rotation())).Rotator();
								QueueFx(FSubjectHandle{ Subject }, FTransform(CombinedRotator, FX.AppearFx.Transform.GetLocation(), FX.AppearFx.Transform.GetScale3D()), FX.AppearFx.SubType);
							}

							// Sound
							if (Appear.bCanPlaySound && Sound.AppearSound)
							{
								QueueSound(Sound.AppearSound);
							}

							// Scale In(WIP)
						}
					}
					else
					{
						Subject.RemoveTraitDeferred<FAppearing>();                                                        
					}

					Appearing.time += DeltaTime;

				}, ThreadsCount, BatchSize);

		Mechanism->ApplyDeferreds();
	}
	#pragma endregion

	// 出生动画
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentAppearAnim");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FAnimation, FAppear, FAppearAnim>();
		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FAnimation& Animation,
				FAppear& Appear,
				FAppearAnim& AppearAnim)
			{
				if (AppearAnim.animTime == 0)
				{
					// 状态机
					Animation.SubjectState = ESubjectState::Appearing;
				}
				else if (AppearAnim.animTime >= Appear.Duration)
				{
					Subject.RemoveTraitDeferred<FAppearAnim>();
				}
				AppearAnim.animTime += DeltaTime;

			}, ThreadsCount, BatchSize);

		Mechanism->ApplyDeferreds();
	}
	#pragma endregion

	// 出生淡入
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentAppearDissolve");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FAppearDissolve, FAnimation,FCurves>();
		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FAnimation& Animation,
				FAppearDissolve& AppearDissolve,
				FCurves& Curves)
			{
				const auto Curve = Curves.DissolveIn.GetRichCurve();
				const auto EndTime = Curve->GetLastKey().Time;
				Animation.Dissolve = 1 - Curve->Eval(FMath::Clamp(AppearDissolve.dissolveTime, 0, EndTime));

				if (AppearDissolve.dissolveTime > EndTime)
				{
					Subject.RemoveTraitDeferred<FAppearDissolve>();
				}

				AppearDissolve.dissolveTime += DeltaTime;

			}, ThreadsCount, BatchSize);

		Mechanism->ApplyDeferreds();
	}
	#pragma endregion

	//----------------------攻击逻辑-------------------------

	// 索敌
	#pragma region
	{
		// 是否索敌（筛选阶段）
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentTrace");

		// 用于存储符合条件的Subject句柄
		TArray<FSolidSubjectHandle, TInlineAllocator<256>> ValidSubjects;
		FCriticalSection ArrayLock;

		FFilter Filter = FFilter::Make<FAgent, FTrace>();
		Filter.Exclude<FAppearing, FDying, FAttacking>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, ThreadsCount, BatchSize);

		// 筛选符合条件的Subjects
		Chain->OperateConcurrently([&](FSolidSubjectHandle Subject, FTrace& Trace)
		{
			bool bShouldTrace = false;

			if (Trace.TimeLeft <= 0)
			{
				Trace.TimeLeft = Trace.CoolDown;
				bShouldTrace = true;
			}
			else
			{
				Trace.TimeLeft -= DeltaTime;
			}

			// 立即设置标记并记录有效句柄
			if (bShouldTrace)
			{
				FScopeLock Lock(&ArrayLock);
				ValidSubjects.Add(Subject);
			}

		}, ThreadsCount, BatchSize);

		// 执行索敌（处理阶段）
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentTracing");

		// 玩家数据准备（同原逻辑）
		bool bPlayerIsValid = false;
		FVector PlayerLocation;
		FSubjectHandle PlayerHandle;
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(CurrentWorld, 0);

		if (IsValid(PlayerPawn))
		{
			USubjectiveActorComponent* SubjectiveComponent = PlayerPawn->FindComponentByClass<USubjectiveActorComponent>();

			if (IsValid(SubjectiveComponent))
			{
				PlayerHandle = SubjectiveComponent->GetHandle();

				if (PlayerHandle.IsValid())
				{
					if (PlayerHandle.HasTrait<FLocated>() && PlayerHandle.HasTrait<FHealth>() && !PlayerHandle.HasTrait<FDying>())
					{
						PlayerLocation = PlayerPawn->GetActorLocation();
						bPlayerIsValid = true;
					}
				}
			}
		}

		// 并行处理筛选结果
		ParallelFor(ValidSubjects.Num(),[&](int32 Index)
		{
			FSolidSubjectHandle Subject = ValidSubjects[Index];
			FLocated& Located = Subject.GetTraitRef<FLocated>();
			FTrace& Trace = Subject.GetTraitRef<FTrace>();
			FCollider& Collider = Subject.GetTraitRef<FCollider>();

			switch (Trace.Mode)
			{
				case ETraceMode::TargetIsPlayer_0:
				{
					Trace.TraceResult = FSubjectHandle{};

					if (bPlayerIsValid)
					{
						// Calculate the distance between Location.Location and PlayerLocation
						float OtherRadius = Trace.TraceResult.HasTrait<FCollider>() ? Trace.TraceResult.GetTrait<FCollider>().Radius : 0;
						float Distance = FMath::Clamp(FVector::Dist(Located.Location, PlayerLocation) - OtherRadius, 0, FLT_MAX);

						// Check if the distance is within Trace.Range
						if (Distance <= Trace.Range)
						{
							Trace.TraceResult = PlayerHandle;
						}
					}
					case ETraceMode::CustomTarget:
					{
						Trace.TraceResult = FSubjectHandle{};

<<<<<<< HEAD
					break;
=======
						AActor* CustomTarget = Trace.CustomTarget.LoadSynchronous();//WIP

						if (IsValid(CustomTarget))
						{
							USubjectiveActorComponent* SubjectiveComponent = CustomTarget->FindComponentByClass<USubjectiveActorComponent>();

							if (IsValid(SubjectiveComponent))
							{
								FSubjectHandle SubjectHandle = SubjectiveComponent->GetHandle();

								if (SubjectHandle.IsValid())
								{
									if (SubjectHandle.HasTrait<FLocated>() && SubjectHandle.HasTrait<FHealth>() && !SubjectHandle.HasTrait<FDying>())
									{
										// Get the location of the custom target
										FVector CustomTargetLocation = CustomTarget->GetActorLocation();

										// Calculate the distance between Location.Location and CustomTargetLocation
										float Distance = FVector::Dist(Located.Location, CustomTargetLocation);

										// Check if the distance is within Trace.Range
										if (Distance <= Trace.Range)
										{
											Trace.TraceResult = SubjectHandle;
										}
									}
								}
							}
						}

						break;
					}
					case ETraceMode::SphereTraceByTraits:
					{
						Trace.TraceResult = FSubjectHandle();

						if (LIKELY(IsValid(Trace.NeighborGrid)))
						{
							FFilter TargetFilter;
							TArray<FSubjectHandle> Results;

							TargetFilter.Include(Trace.IncludeTraits);
							TargetFilter.Exclude(Trace.ExcludeTraits);

							Trace.NeighborGrid->SphereExpandForSubjects(Located.Location, Trace.Range, Collider.Radius, TargetFilter, Results);

							if (!Results.IsEmpty())
							{
								if (Results[0].IsValid())
								{
									Trace.TraceResult = Results[0];
								}
							}
						}

						break;
					}
>>>>>>> parent of 0f9a801 (Beta.2)
				}
				case ETraceMode::SphereTraceByTraits:
				{
					Trace.TraceResult = FSubjectHandle();

					if (LIKELY(IsValid(Trace.NeighborGrid)))
					{
						FFilter TargetFilter;
						FSubjectHandle Result;

						TargetFilter.Include(Trace.IncludeTraits);
						TargetFilter.Exclude(Trace.ExcludeTraits);

						Trace.NeighborGrid->SphereExpandForSubject(Located.Location, Trace.Range, Collider.Radius, TargetFilter, Result);

						if (Result.IsValid())
						{
							Trace.TraceResult = Result;
						}
					}

					break;
				}
			}

		},EParallelForFlags::BackgroundPriority | EParallelForFlags::Unbalanced );
	}
	#pragma endregion

	// 攻击触发 
	#pragma region 
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentAttackMain");

		FFilter Filter = FFilter::Make<FAgent, FAttack, FRendering, FLocated, FDirected, FTrace, FCollider>();
		Filter.Exclude<FAppearing, FDying, FAttacking>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				const FLocated& Located,
				FDirected Directed,
				FAttack& Attack,
				FTrace& Trace,
				FCollider& Collider)
			{
				if (!Attack.bEnable) { return; }

				if (Trace.TraceResult.IsValid())
				{
					if (Trace.TraceResult.HasTrait<FLocated>() && Trace.TraceResult.HasTrait<FHealth>())
					{
						float TargetHealth = Trace.TraceResult.GetTrait<FHealth>().Current;

						FVector TargetLocation = Trace.TraceResult.GetTrait<FLocated>().Location;
						float OtherRadius = Trace.TraceResult.HasTrait<FCollider>() ? Trace.TraceResult.GetTrait<FCollider>().Radius : 0;
						float DistToTarget = FMath::Clamp(FVector::Dist(Located.Location, TargetLocation) - OtherRadius, 0, FLT_MAX);

						FRotator Delta = Directed.Direction.Rotation() - (TargetLocation - Located.Location).Rotation();
						float DeltaYaw = FMath::Abs(Delta.Yaw);
						float AllowedYaw = 5;

						switch (Attack.AttackMode)
						{
							case EAttackMode::Melee:
								AllowedYaw = 30;
								break;
							case EAttackMode::Ranged:
								AllowedYaw = 5;
								break;
							case EAttackMode::Suicide:
								AllowedYaw = 90;
								break;
						}

						// 攻击距离内触发攻击
						if (DistToTarget <= Attack.Range && DistToTarget <= Trace.Range && DeltaYaw <= AllowedYaw && TargetHealth > 0)
						{
							Subject.SetTraitDeferred(FAttacking{ 0.f, EAttackState::PreCast });
						}
					}
				}
			}, ThreadsCount, BatchSize);

		Mechanism->ApplyDeferreds();
	}
	#pragma endregion

	// 攻击过程
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentAttacking");

		FFilter Filter = FFilter::Make<FAgent, FAttack, FRendering, FLocated, FAnimation, FAttacking, FMove, FMoving, FDirected, FSound, FFX, FTrace, FDebuff, FDamage, FSpawnActor>();
		Filter.Exclude<FAppearing, FDying>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FLocated& Located,
				FRendering& Rendering,
				FAnimation& Animation,
				FAttack& Attack,
				FAttacking& Attacking,
				FSound& Sound,
				FMove& Move,
				FMoving& Moving,
				FDirected& Directed,
				FFX& FX,
				FTrace& Trace,
				FDebuff& Debuff,
				FDamage& Damage,
				FSpawnActor& SpawnActor)
			{
				TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentAttackMeelee_Body");

				if (Attack.AttackMode == EAttackMode::Suicide)
				{
					// Fx
					if (Attack.bCanSpawnFx && FX.AttackFx.SubType != ESubType::None)
					{
						FRotator CombinedRotator = (FQuat(FX.AttackFx.Transform.GetRotation().Rotator()) * FQuat(Directed.Direction.Rotation())).Rotator();
						QueueFx(FSubjectHandle{ Subject }, FTransform(CombinedRotator, FX.AttackFx.Transform.GetLocation(), FX.AttackFx.Transform.GetScale3D()), FX.AttackFx.SubType);
					}

					// Sound
					if (Attack.bCanPlaySound && Sound.AttackSound)
					{
						QueueSound(Sound.AttackSound);
					}

					// Deal Damage
					if (Trace.TraceResult.IsValid() && !Trace.TraceResult.HasTrait<FDying>())
					{
						FDmgSphere DmgSphere = { Damage.Damage,Damage.KineticDmg,Damage.FireDmg,Damage.IceDmg,Damage.PercentDmg,Damage.CritProbability,Damage.CritMult };
						ApplyDamageToSubjects(TArray<FSubjectHandle>{Trace.TraceResult}, TArray<FSubjectHandle>{}, FSubjectHandle{}, Located.Location, DmgSphere, Debuff);
					}

					// Die
					Subject.DespawnDeferred();
				}
				else
				{
					// First execute
					if (UNLIKELY(Attacking.Time == 0))
					{
						Animation.SubjectState = ESubjectState::Attacking;
						Animation.PreviousSubjectState = ESubjectState::Dirty;

						// Danger Warning Decal
						if (Attack.bCanSpawnDangeWarning && SpawnActor.DangerWarningClass)
						{
							FVector ForwardVector = Subject.GetTraitRef<FDirected, EParadigm::Unsafe>().Direction.GetSafeNormal(); // 获取Subject的前向向量并规范化
							FRotator ForwardRotation = ForwardVector.Rotation(); // 获取前向向量对应的旋转量

							// 将投射物起始偏移转换到Subject的本地坐标系
							FVector DangerWarningLocalOffset = ForwardRotation.RotateVector(SpawnActor.DangerWarningOffset);

							FSpawningActor DangerWarningToSpawn;
							DangerWarningToSpawn.SpawnClass = SpawnActor.DangerWarningClass;
							DangerWarningToSpawn.Trans = FTransform(ForwardRotation, FLocated{ Located.Location + DangerWarningLocalOffset }, FVector::OneVector);

							FSubjectRecord DangerWarningRecord;
							DangerWarningRecord.SetTrait(DangerWarningToSpawn);

							Mechanism->SpawnSubjectDeferred(DangerWarningRecord);
						}
					}

					// 到达造成伤害的时间点并且本轮之前也没有攻击过，造成一次伤害
					if (Attacking.Time >= Attack.TimeOfAttack && Attacking.Time < Attack.DurationPerRound)
					{
						if (Attacking.State == EAttackState::PreCast)
						{
							Attacking.State = EAttackState::PostCast;

							// 特效
							if (Attack.bCanSpawnFx && FX.AttackFx.SubType != ESubType::None)
							{
								FRotator CombinedRotator = (FQuat(FX.AttackFx.Transform.GetRotation()) * FQuat(Directed.Direction.Rotation())).Rotator();
								QueueFx(FSubjectHandle{ Subject }, FTransform(CombinedRotator, FX.AttackFx.Transform.GetLocation(), FX.AttackFx.Transform.GetScale3D()), FX.AttackFx.SubType);
							}

							// 音效
							if (Attack.bCanPlaySound && Sound.AttackSound)
							{
								QueueSound(Sound.AttackSound);
							}

							// 近战需要满足距离和角度条件才能造成伤害
							if (Attack.AttackMode == EAttackMode::Melee && Trace.TraceResult.IsValid() && !Trace.TraceResult.HasTrait<FDying>())
							{
								// 获取双方位置信息
								FVector AttackerPos = Located.Location;
								FVector TargetPos = Trace.TraceResult.GetTraitRef<FLocated, EParadigm::Unsafe>().Location;

								// 计算距离
								float OtherRadius = Trace.TraceResult.HasTrait<FCollider>() ? Trace.TraceResult.GetTrait<FCollider>().Radius : 0;
								float Distance = FMath::Clamp(FVector::Distance(AttackerPos, TargetPos) - OtherRadius, 0, FLT_MAX);

								// 计算夹角
								FVector AttackerForward = Subject.GetTraitRef<FDirected, EParadigm::Unsafe>().Direction.GetSafeNormal();
								FVector ToTargetDir = (TargetPos - AttackerPos).GetSafeNormal();
								float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(AttackerForward, ToTargetDir)));

								// 双重条件判断
								if (Distance <= Attack.Range && Angle <= Attack.MeleeAngle)
								{
									FDmgSphere DmgSphere = { Damage.Damage,Damage.KineticDmg,Damage.FireDmg,Damage.IceDmg,Damage.PercentDmg,Damage.CritProbability,Damage.CritMult };
									ApplyDamageToSubjects(TArray<FSubjectHandle>{Trace.TraceResult}, TArray<FSubjectHandle>{}, FSubjectHandle{ Subject }, Located.Location, DmgSphere, Debuff);
								}
							}

							// 如果是远程攻击就生成子弹
							if (Attack.AttackMode == EAttackMode::Ranged && Attack.bCanSpawnProjectile && SpawnActor.ProjectileClass && Trace.TraceResult.IsValid())
							{
								FVector ForwardVector = Subject.GetTraitRef<FDirected, EParadigm::Unsafe>().Direction.GetSafeNormal(); // 获取Subject的前向向量并规范化
								FRotator ForwardRotation = ForwardVector.Rotation(); // 获取前向向量对应的旋转量

								// 将投射物起始偏移转换到Subject的本地坐标系
								FVector ProjectileLocalOffset = ForwardRotation.RotateVector(SpawnActor.ProjectileOffset);

								FSpawningActor ProjectileToSpawn;
								ProjectileToSpawn.SpawnClass = SpawnActor.ProjectileClass;
								ProjectileToSpawn.Trans = FTransform(ForwardRotation, FLocated{ Located.Location + ProjectileLocalOffset }, FVector::OneVector);

								FSubjectRecord ProjectileRecord;
								ProjectileRecord.SetTrait(ProjectileToSpawn);

								Mechanism->SpawnSubjectDeferred(ProjectileRecord);
							}
						}
					}

					// 冷却等待下一轮攻击
					else if (Attacking.Time >= Attack.DurationPerRound && Attacking.Time < Attack.DurationPerRound + Attack.CoolDown)
					{
						Attacking.State = EAttackState::Cooling;
						Animation.SubjectState = ESubjectState::Idle;
					}

					// 到达计时器时间，本轮攻击结束
					else if (Attacking.Time >= Attack.DurationPerRound + Attack.CoolDown)
					{
						Subject.RemoveTraitDeferred<FAttacking>();// 移除攻击状态
						Moving.KnockBackForce = FVector::ZeroVector; // 击退力清零
					}

					Attacking.Time += DeltaTime;
				}
			}, ThreadsCount, BatchSize);

		Mechanism->ApplyDeferreds();
	}
	#pragma endregion

	//----------------------受击逻辑-------------------------
	
	// 受击发光
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentHitGlow");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FHitGlow, FAnimation, FCurves>();
		Filter.Exclude<FAppearing, FBeingHit>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FAnimation& Animation,
				FHitGlow& HitGlow,
				FCurves& Curves)
			{
				// 获取曲线
				auto Curve = Curves.HitEmission.GetRichCurve();

				// 检查曲线是否有关键帧
				if (Curve->GetNumKeys() == 0)
				{
					// 如果没有关键帧，添加默认关键帧
					Curve->Reset();
					FKeyHandle Key1 = Curve->AddKey(0.0f, 0.0f);    // 初始值
					FKeyHandle Key2 = Curve->AddKey(0.1f, 1.0f);   // 峰值
					FKeyHandle Key3 = Curve->AddKey(0.5f, 0.0f);   // 结束值

					// 设置自动切线
					Curve->SetKeyInterpMode(Key1, RCIM_Cubic);
					Curve->SetKeyInterpMode(Key2, RCIM_Cubic);
					Curve->SetKeyInterpMode(Key3, RCIM_Cubic);

					Curve->SetKeyTangentMode(Key1, RCTM_Auto);
					Curve->SetKeyTangentMode(Key2, RCTM_Auto);
					Curve->SetKeyTangentMode(Key3, RCTM_Auto);
				}

				// 获取曲线的最后一个关键帧的时间
				const auto EndTime = Curve->GetLastKey().Time;

				// 受击发光
				Animation.HitGlow = Curve->Eval(HitGlow.GlowTime);

				// 更新发光时间
				if (HitGlow.GlowTime < EndTime)
				{
					HitGlow.GlowTime += DeltaTime;
				}

				// 计时器完成后删除 Trait
				if (HitGlow.GlowTime >= EndTime)
				{
					Animation.HitGlow = 0; // 重置发光值
					Subject->RemoveTraitDeferred<FHitGlow>(); // 延迟删除 Trait
				}

			}, ThreadsCount, BatchSize);

		Mechanism->ApplyDeferreds();
}
	#pragma endregion

	// 怪物受击形变
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentSqueezeSquash");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FSqueezeSquash, FScaled, FHit, FCurves>();
		Filter.Exclude<FAppearing, FBeingHit>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FScaled& Scaled,
				FSqueezeSquash& SqueezeSquash,
				FHit& Hit,
				FCurves& Curves)
			{
				// 获取曲线
				auto Curve = Curves.HitSqueezeSquash.GetRichCurve();

				// 检查曲线是否有关键帧
				if (Curve->GetNumKeys() == 0)
				{
					// 如果没有关键帧，添加默认关键帧
					Curve->Reset();
					FKeyHandle Key1 = Curve->AddKey(0.0f, 1.0f);    // 初始值
					FKeyHandle Key2 = Curve->AddKey(0.1f, 1.75f);   // 峰值
					FKeyHandle Key3 = Curve->AddKey(0.28f, 0.78f);   // 回弹
					FKeyHandle Key4 = Curve->AddKey(0.4f, 1.12f);   // 二次回弹
					FKeyHandle Key5 = Curve->AddKey(0.5f, 1.0f);     // 恢复

					// 设置自动切线
					Curve->SetKeyInterpMode(Key1, RCIM_Cubic);
					Curve->SetKeyInterpMode(Key2, RCIM_Cubic);
					Curve->SetKeyInterpMode(Key3, RCIM_Cubic);
					Curve->SetKeyInterpMode(Key4, RCIM_Cubic);
					Curve->SetKeyInterpMode(Key5, RCIM_Cubic);

					Curve->SetKeyTangentMode(Key1, RCTM_Auto);
					Curve->SetKeyTangentMode(Key2, RCTM_Auto);
					Curve->SetKeyTangentMode(Key3, RCTM_Auto);
					Curve->SetKeyTangentMode(Key4, RCTM_Auto);
					Curve->SetKeyTangentMode(Key5, RCTM_Auto);
				}

				// 获取曲线的最后一个关键帧的时间
				const auto EndTime = Curve->GetLastKey().Time;

				// 受击变形
				Scaled.renderFactors.X = FMath::Lerp(Scaled.Factors.X, Scaled.Factors.X * Curve->Eval(SqueezeSquash.squeezeSquashTime), Hit.SqueezeSquashStr);
				Scaled.renderFactors.Y = FMath::Lerp(Scaled.Factors.Y, Scaled.Factors.Y * Curve->Eval(SqueezeSquash.squeezeSquashTime), Hit.SqueezeSquashStr);
				Scaled.renderFactors.Z = FMath::Lerp(Scaled.Factors.Z, Scaled.Factors.Z * (2.f - Curve->Eval(SqueezeSquash.squeezeSquashTime)), Hit.SqueezeSquashStr);

				// 更新形变时间
				if (SqueezeSquash.squeezeSquashTime < EndTime)
				{
					SqueezeSquash.squeezeSquashTime += DeltaTime;
				}

				// 计时器完成后删除 Trait
				if (SqueezeSquash.squeezeSquashTime >= EndTime)
				{
					Scaled.renderFactors = Scaled.Factors; // 恢复原始比例
					Subject->RemoveTraitDeferred<FSqueezeSquash>(); // 延迟删除 Trait
				}

			}, ThreadsCount, BatchSize);

		Mechanism->ApplyDeferreds();
	}
	#pragma endregion

	// 灼烧持续掉血
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentBurning");

		auto Filter = FFilter::Make<FTemporalDamaging>();
		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject, FTemporalDamaging& Temporal)
			{
				// 持续伤害结束或玩家退出，终止执行
				if (Temporal.RemainingTemporalDamage <= 0)
				{
					// 恢复颜色
					if (Temporal.TemporalDamageTarget.HasTrait<FAnimation>())
					{
						auto& TargetAnimation = Temporal.TemporalDamageTarget.GetTraitRef<FAnimation, EParadigm::Unsafe>();
						TargetAnimation.BurnFx = 0;
					}

					Subject.DespawnDeferred();
					return;
				}

				Temporal.TemporalDamageTimeout -= DeltaTime;

				// 倒计时，每0.5秒重置，即每秒造成2段灼烧伤害
				if (Temporal.TemporalDamageTimeout <= 0)
				{
					// 扣除目标生命值
					if (Temporal.TemporalDamageTarget.HasTrait<FHealth>())
					{
						auto& TargetHealth = Temporal.TemporalDamageTarget.GetTraitRef<FHealth, EParadigm::Unsafe>();

						if (TargetHealth.Current > 0)
						{
							// 实际伤害值
							float ClampedDamage = FMath::Min(Temporal.TotalTemporalDamage * 0.25f, TargetHealth.Current);

							// 应用伤害
							TargetHealth.DamageToTake.Enqueue(ClampedDamage);

							// 记录伤害施加者
							TargetHealth.DamageInstigator.Enqueue(Temporal.TemporalDamageInstigator);

							// 生成伤害数字
							if (Temporal.TemporalDamageTarget.HasTrait<FTextPopUp>())
							{
								const auto& TextPopUp = Temporal.TemporalDamageTarget.GetTraitRef<FTextPopUp, EParadigm::Unsafe>();

								if (TextPopUp.Enable)
								{
									float Style;

									if (ClampedDamage < TextPopUp.WhiteTextBelowPercent)
									{
										Style = 0;
									}
									else if (ClampedDamage < TextPopUp.OrangeTextAbovePercent)
									{
										Style = 1;
									}
									else
									{
										Style = 2;
									}

									float Radius = 0;

									if (Temporal.TemporalDamageTarget.HasTrait<FCollider>())
									{
										Radius = Temporal.TemporalDamageTarget.GetTraitRef<FCollider, EParadigm::Unsafe>().Radius;
									}

									FVector Location = FVector::ZeroVector;

									if (Temporal.TemporalDamageTarget.HasTrait<FLocated>())
									{
										Location = Temporal.TemporalDamageTarget.GetTraitRef<FLocated, EParadigm::Unsafe>().Location;
									}

									Location + FVector(0, 0, Radius);

									QueueText(Temporal.TemporalDamageTarget, ClampedDamage, Style, TextPopUp.TextScale, Radius * 1.1, Location);
								}
							}

							// 让目标材质变红
							if (Temporal.TemporalDamageTarget.HasTrait<FAnimation>())
							{
								auto& TargetAnimation = Temporal.TemporalDamageTarget.GetTraitRef<FAnimation, EParadigm::Unsafe>();
								TargetAnimation.Lock();
								TargetAnimation.BurnFx = 1;
								TargetAnimation.Unlock();
							}
						}
					}

					// 4段伤害都释放完毕就删除 dummy subject
					Temporal.RemainingTemporalDamage -= Temporal.TotalTemporalDamage * 0.25f;

					if (Temporal.RemainingTemporalDamage > 0.f)
					{
						Temporal.TemporalDamageTimeout = 0.5f;
					}
				}
			}, ThreadsCount, BatchSize);

			Mechanism->ApplyDeferreds();
	}
	#pragma endregion

	// 结算伤害
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("DecideAgentDamage");

		FFilter Filter = FFilter::Make<FHealth, FLocated>();// it processes hero and prop type too
		Filter.Exclude<FAppearing, FDying>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FHealth& Health,
				FLocated& Located)
			{
				while (!Health.DamageToTake.IsEmpty() && !Health.DamageInstigator.IsEmpty())
				{
					// 如果怪物死了，跳出循环
					if (Health.Current <= 0) { break; }

					FSubjectHandle Instigator;
					float damageToTake;

					// Queue为Mpsc不需要锁
					Health.DamageInstigator.Dequeue(Instigator);
					Health.DamageToTake.Dequeue(damageToTake);

					bool bIsValidStats = false;
					FStatistics* Stats = nullptr;

					if (Instigator.IsValid())
					{
						if (Instigator.HasTrait<FStatistics>())
						{
							// 伤害与积分最后结算
							Stats = Instigator.GetTraitPtr<FStatistics, EParadigm::Unsafe>();

							if (Stats->bEnable)
							{
								bIsValidStats = true;
							}
						}
					}

					if (Health.Current - damageToTake > 0) // 不是致命伤害
					{
						// 统计数据
						if (bIsValidStats)
						{
							Stats->Lock();
							Stats->totalDamage += damageToTake;
							Stats->Unlock();
						}
					}
					else // 是致命伤害
					{
						Subject.SetTraitDeferred(FDying{ 0,0,Instigator });	// 标记为死亡

						if (Subject.HasTrait<FMove>())
						{
							Subject.GetTraitRef<FMove,EParadigm::Unsafe>().bCanFly = false; // 如果在飞行会掉下来
						}

						// 统计数据
						if (bIsValidStats)
						{
							int32 Score = 0;

							if (Subject.HasTrait<FAgent>())
							{
								Score = Subject.GetTrait<FAgent>().Score;
							}

							Stats->Lock();
							Stats->totalDamage += FMath::Min(damageToTake, Health.Current);
							Stats->totalKills += 1;
							Stats->totalScore += Score;
							Stats->Unlock();
						}
					}

					// 扣除血量
					Health.Current -= FMath::Min(damageToTake, Health.Current);
				}
			}, ThreadsCount, BatchSize);

		Mechanism->ApplyDeferreds();
	}
	#pragma endregion

	// 更新血条
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentHealthBar");

		static const auto Filter = FFilter::Make<FAgent, FRendering, FHealth, FHealthBar>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FHealth Health,
				FHealthBar& HealthBar)
			{
				if (HealthBar.bShowHealthBar)
				{
					HealthBar.TargetRatio = FMath::Clamp(Health.Current / Health.Maximum, 0, 1);
					HealthBar.CurrentRatio = FMath::FInterpTo(HealthBar.CurrentRatio, HealthBar.TargetRatio, DeltaTime, HealthBar.InterpSpeed);// WIP use niagara instead

					if (HealthBar.HideOnFullHealth)
					{
						if (Health.Current == Health.Maximum)
						{
							HealthBar.Opacity = 0;
						}
						else
						{
							HealthBar.Opacity = 1;
						}
					}
					else
					{
						HealthBar.Opacity = 1;
					}

					if (HealthBar.HideOnEmptyHealth && Health.Current <= 0)
					{
						HealthBar.Opacity = 0;
					}
				}
				else
				{
					HealthBar.Opacity = 0;
				}
			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	//----------------------死亡逻辑-------------------------

	// 死亡总
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentDeathMain");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FDeath, FSound, FLocated, FDying, FDirected, FFX, FTrace, FMove, FMoving, FSpawnActor>();
		Filter.Exclude<FAppearing>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FDeath Death,
				FSound Sound,
				FLocated Located,
				FDying& Dying,
				FDirected Directed,
				FFX FX,
				FTrace& Trace,
				FMove& Move,
				FMoving& Moving,
				FSpawnActor& SpawnActor)
			{
				if (Dying.Time == 0)
				{
					Moving.Speed = 0;
					Dying.Duration = Death.DespawnDelay;

					// Stop attacking
					if (Subject.HasTrait<FAttacking>())
					{
						Subject.RemoveTraitDeferred<FAttacking>();
					}

					// Drop loot
					if (Death.NumSpawnLoot > 0 && SpawnActor.DeathSpawnClass)
					{
						FSpawningActor LootToSpawn;
						LootToSpawn.SpawnClass = SpawnActor.DeathSpawnClass;
						LootToSpawn.Quantity = Death.NumSpawnLoot;
						LootToSpawn.Trans = FTransform(FRotator::ZeroRotator, FLocated{ Located }, SpawnActor.DeathSpawnScale);

						FSubjectRecord Record;
						Record.SetTrait(LootToSpawn);

						Mechanism->SpawnSubjectDeferred(Record);
					}

					// Fade out
					if (Death.bCanFadeout)
					{
						Subject.SetTraitDeferred(FDeathDissolve{});
					}

					// Anim
					if (Death.bCanPlayAnim)
					{
						Subject.SetTraitDeferred(FDeathAnim{});
					}

					// Sound
					if (Sound.DeathSound)
					{
						QueueSound(Sound.DeathSound);
					}

					// Scale In(WIP)
				}
				else if (Dying.Time > Dying.Duration)
				{
					FVector InstigatorLocation = Located.Location;
					FVector Direction = Directed.Direction;

					if (Dying.Instigator.HasTrait<FLocated>() && !Dying.Instigator.HasTrait<FDying>())
					{
						InstigatorLocation = Dying.Instigator.GetTrait<FLocated>().Location;
						Direction = (Located.Location - InstigatorLocation).GetSafeNormal2D();
					}
					
					// Fx
					if (Death.bCanSpawnFx && FX.DeathFx.SubType != ESubType::None)
					{
						FRotator CombinedRotator = (FQuat(FX.DeathFx.Transform.GetRotation()) * FQuat(Direction.Rotation())).Rotator();
						QueueFx(FSubjectHandle{ Subject }, FTransform(CombinedRotator, FX.DeathFx.Transform.GetLocation(), FX.DeathFx.Transform.GetScale3D()), FX.DeathFx.SubType);
					}

					// 移除	
					Subject.DespawnDeferred();
				}

				Dying.Time += DeltaTime;

			}, ThreadsCount, BatchSize);

		Mechanism->ApplyDeferreds();
	}
	#pragma endregion

	// 死亡消融
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentDeathDissolve");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FDeathDissolve, FAnimation, FDying, FDeath, FCurves>();
		Filter.Exclude<FAppearing>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FAnimation& Animation,
				FDeathDissolve& DeathDissolve,
				FDeath& Death,
				FCurves& Curves)
			{
				// 获取曲线
				auto Curve = Curves.DissolveOut.GetRichCurve();

				// 检查曲线是否有关键帧
				if (Curve->GetNumKeys() == 0)
				{
					// 如果没有关键帧，添加默认关键帧
					Curve->Reset();
					FKeyHandle Key1 = Curve->AddKey(0.0f, 1.0f); // 初始值
					FKeyHandle Key2 = Curve->AddKey(1.0f, 0.0f); // 结束值

					// 设置自动切线
					Curve->SetKeyInterpMode(Key1, RCIM_Cubic);
					Curve->SetKeyInterpMode(Key2, RCIM_Cubic);

					Curve->SetKeyTangentMode(Key1, RCTM_Auto);
					Curve->SetKeyTangentMode(Key2, RCTM_Auto);
				}

				// 获取曲线的最后一个关键帧的时间
				const auto EndTime = Curve->GetLastKey().Time;

				// 计算溶解效果
				if (DeathDissolve.dissolveTime >= Death.FadeOutDelay && (DeathDissolve.dissolveTime - Death.FadeOutDelay) < EndTime)
				{
					Animation.Dissolve = 1 - Curve->Eval(DeathDissolve.dissolveTime - Death.FadeOutDelay);
				}

				// 更新溶解时间
				DeathDissolve.dissolveTime += DeltaTime;

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 死亡动画
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentDeathAnim");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FDeathAnim, FAnimation, FDying>();
		Filter.Exclude<FAppearing>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FAnimation& Animation,
				FDeathAnim& DeathAnim)
			{
				if (DeathAnim.animTime == 0)
				{
					Animation.SubjectState = ESubjectState::Dying;
					Animation.PreviousSubjectState = ESubjectState::Dirty;
				}

				DeathAnim.animTime += DeltaTime;

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	//-----------------------移动逻辑------------------------

	// 冰冻减速
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentFrozen");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FAnimation, FFreezing>();
		Filter.Exclude<FAppearing, FDying>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FAnimation& Animation,
				FFreezing& Freezing)
			{
				// 更新计时器
				if (Freezing.FreezeTimeout > 0.f)
				{
					Freezing.FreezeTimeout -= DeltaTime;
				}

				// 计时器结束
				if (Freezing.FreezeTimeout <= 0.f)
				{
					// 设置DP为0，怪物颜色复原
					Animation.FreezeFx = 0;
					Animation.PreviousSubjectState = ESubjectState::Dirty; // 强制刷新动画状态机

					// 怪物可以解耦
					Subject.RemoveTraitDeferred<FFreezing>();
				}

			}, ThreadsCount, BatchSize);

		Mechanism->ApplyDeferreds();
	}
	#pragma endregion

	// Speed Limit Override
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("SpeedLimitOverride");

		FFilter Filter = FFilter::Make<FCollider, FLocated, FRoadBlock>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FCollider Collider,
				FLocated Located,
				FRoadBlock& RoadBlock)
			{
				TArray<FSubjectHandle> Results;

				if (UNLIKELY(!IsValid(RoadBlock.NeighborGrid))) return;

				if (!RoadBlock.bOverrideSpeedLimit) return;

				RoadBlock.NeighborGrid->SphereTraceForSubjects(Located.Location, Collider.Radius, FFilter::Make<FAgent, FLocated, FCollider, FMoving>(), Results);

				if (!Results.IsEmpty())
				{
					RoadBlock.OverridingAgents.Append(Results);
				}

				if (RoadBlock.OverridingAgents.IsEmpty()) return;

				TSet<FSubjectHandle> Agents = RoadBlock.OverridingAgents;

				for (const auto& Agent : Agents)
				{
					if (UNLIKELY(!Agent.IsValid())) continue;

					float AgentRadius = Agent.GetTrait<FCollider>().Radius;
					float RoadBlockRadius = Collider.Radius;
					float CombinedRadius = AgentRadius + RoadBlockRadius;
					FVector AgentLocation = Agent.GetTrait<FLocated>().Location;
					float Distance = FVector::Distance(Located.Location, AgentLocation);

					FMoving& AgentMoving = Agent.GetTraitRef<FMoving, EParadigm::Unsafe>();

					if (Distance <  CombinedRadius)
					{
						AgentMoving.Lock();
						AgentMoving.bPushedBack = true;
						AgentMoving.PushBackSpeedOverride = RoadBlock.NewSpeedLimit;
						AgentMoving.Unlock();
					}
					else if(Distance > CombinedRadius * 1.25f)
					{
						AgentMoving.Lock();
						AgentMoving.bPushedBack = false;
						AgentMoving.Unlock();
						RoadBlock.OverridingAgents.Remove(Agent);
					}
				}
			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 水平运动
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentMovement");

<<<<<<< HEAD
		FFilter Filter = FFilter::Make<FAgent, FRendering, FAnimation, FMove, FMoving, FDirected, FLocated, FAttack, FTrace, FNavigation, FAvoidance, FCollider, FDefence>();
=======
		// 初始化过滤器
		FFilter Filter = FFilter::Make<FAgent, FRendering, FAnimation, FMove, FMoving, FDirected, FLocated, FAttack, FTrace, FNavigation,FAvoidance>();
>>>>>>> parent of 0f9a801 (Beta.2)
		Filter.Exclude<FAppearing>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FAnimation& Animation,
				FMove& Move,
				FMoving& Moving,
				FDirected& Directed,
				FLocated& Located,
				FAttack& Attack,
				FTrace& Trace,
				FNavigation& Navigation,
				FAvoidance& Avoidance)
			{
				if (!Move.bEnable) return;

				// 位置和方向初始化
<<<<<<< HEAD
				FVector& AgentLocation = Located.Location;
				FVector GoalLocation = AgentLocation;

				FVector DesiredMoveDirection = Directed.Direction;
				Moving.SpeedMult = 1;

				bool bInside_BaseFF = false;
				FCellStruct Cell_BaseFF = FCellStruct{};

				bool bInside_TargetFF = false;
				FCellStruct Cell_TargetFF = FCellStruct{};

				const bool bIsAttacking = Subject.HasTrait<FAttacking>();
				const bool bIsFreezing = Subject.HasTrait<FFreezing>();
				const bool bIsDying = Subject.HasTrait<FDying>();
=======
				const FVector& AgentLocation = Located.Location;
				FVector DesiredDirection = Directed.Direction;
				FVector GoalLocation = AgentLocation;
				float DesiredSpeed = Move.Speed;
				ERotationMode RotationMode = Move.RotationMode;
>>>>>>> parent of 0f9a801 (Beta.2)

				//--------------------------- 目标点 --------------------------//

<<<<<<< HEAD
				// Lambda to handle direct approach logic WIP add individual navigation in here
				auto TryApproachDirectly = [&]()
				{
					if (Trace.TraceResult.HasTrait<FLocated>())
					{
						GoalLocation = Trace.TraceResult.GetTraitRef<FLocated, EParadigm::Unsafe>().Location;
						DesiredMoveDirection = (GoalLocation - AgentLocation).GetSafeNormal2D();
					}
					else
					{
						Moving.SpeedMult = 0;
					}
				};

				if (IsValid(Navigation.FlowField)) // 默认流场, 必须获取因为之后要用到地面高度
				{
					Navigation.FlowField->GetCellAtLocation(AgentLocation, bInside_BaseFF, Cell_BaseFF);

					if (bInside_BaseFF)
					{
						GoalLocation = Navigation.FlowField->goalLocation;
						DesiredMoveDirection = Cell_BaseFF.dir.GetSafeNormal2D();
					}
					else
					{
						Moving.SpeedMult = 0;
					}
				}

				if (Trace.TraceResult.IsValid()) // 有目标，目标有指向目标自己的流场
=======
				if (LIKELY(Trace.TraceResult.IsValid())) // 追踪目标优先
>>>>>>> parent of 0f9a801 (Beta.2)
				{
					if (LIKELY(Trace.TraceResult.HasTrait<FLocated>()))
					{
<<<<<<< HEAD
						FBindFlowField BindFlowField = Trace.TraceResult.GetTrait<FBindFlowField>();

						if (IsValid(BindFlowField.FlowField)) // 从目标获取指向目标的流场
						{
							BindFlowField.FlowField->GetCellAtLocation(AgentLocation, bInside_TargetFF, Cell_TargetFF);

							if (bInside_TargetFF)
							{
								GoalLocation = BindFlowField.FlowField->goalLocation;
								DesiredMoveDirection = Cell_TargetFF.dir.GetSafeNormal2D();
							}
							else
							{
								TryApproachDirectly();
							}
						}
						else
						{
							TryApproachDirectly();
						}
=======
						RotationMode = ERotationMode::VelocityFollowRotation;
						GoalLocation = Trace.TraceResult.GetTrait<FLocated>().Location;
						DesiredDirection = (GoalLocation - AgentLocation).GetSafeNormal2D();
>>>>>>> parent of 0f9a801 (Beta.2)
					}
					else
					{
						DesiredSpeed = 0; // 获取不到目标位置，这是误操作，不移动
					}
				}
<<<<<<< HEAD

				//--------------------------- 计算水平速度 ----------------------------//

				// 助推力
				if (Moving.bLaunching)
				{
					Moving.CurrentVelocity += Moving.LaunchForce;

					FVector XYDir = Moving.CurrentVelocity.GetSafeNormal2D();
					float XYSpeed = Moving.CurrentVelocity.Size2D();
					XYSpeed = FMath::Clamp(XYSpeed, 0, Move.MoveSpeed + Defence.KineticMaxImpulse);
					FVector XYVelocity = XYSpeed * XYDir;

					float ZSpeed = Moving.CurrentVelocity.Z;
					ZSpeed = FMath::Clamp(ZSpeed, -Defence.KineticMaxImpulse, Defence.KineticMaxImpulse);

					Moving.CurrentVelocity = FVector(XYVelocity.X, XYVelocity.Y, ZSpeed);
					Moving.LaunchTimer = Moving.CurrentVelocity.Size2D() / Move.Deceleration_Ground;

					Moving.LaunchForce = FVector::ZeroVector;
					Moving.bLaunching = false;
=======
				else if (LIKELY(IsValid(Navigation.FlowField))) // 有流场就按流场走
				{
					bool bInside;
					FCellStruct Cell;
					Navigation.FlowField->GetCellAtLocation(AgentLocation, bInside, Cell);

					if (LIKELY(bInside))
					{
						GoalLocation = Navigation.FlowField->goalLocation;
						DesiredDirection = Cell.dir.GetSafeNormal2D();
					}
					else
					{
						DesiredSpeed = 0;// 在流场外面，流场不够大，是误操作，不移动
					}
>>>>>>> parent of 0f9a801 (Beta.2)
				}
				else // 既没有目标，又没有流场，不移动
				{
					DesiredSpeed = 0;
				}

<<<<<<< HEAD
				// 夹角
				const FVector CurrentDir2D = Moving.CurrentVelocity.GetSafeNormal2D();
				const FVector DesiredDir2D = Moving.DesiredVelocity.GetSafeNormal2D();
				const float DotProduct = FVector::DotProduct(CurrentDir2D, DesiredDir2D);
				const float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

				// 速度-夹角曲线
				const TRange<float> TurnInputRange(Move.TurnSpeedRangeMap.Y, Move.TurnSpeedRangeMap.W);
				const TRange<float> TurnOutputRange(Move.TurnSpeedRangeMap.X, Move.TurnSpeedRangeMap.Z);
				Moving.SpeedMult *= FMath::GetMappedRangeValueClamped(TurnInputRange, TurnOutputRange, AngleDegrees);
				
				// 朝向
				if (UNLIKELY(bIsDying) || UNLIKELY(Moving.bFalling))
				{
					// 保持当前方向
=======
				//--------------------------- 移动方向计算 ------------------------//

				const bool bIsAttacking = Subject.HasTrait<FAttacking>();
				const bool bIsFreezing = Subject.HasTrait<FFreezing>();
				const bool bIsDying = Subject.HasTrait<FDying>();

				if (UNLIKELY(Moving.bKnockedBack || bIsDying))
				{
					// 被击退或死亡时保持方向
					Moving.Direction = Directed.Direction;
>>>>>>> parent of 0f9a801 (Beta.2)
				}
				else if (UNLIKELY(bIsAttacking))
				{
					if (Subject.GetTrait<FAttacking>().State != EAttackState::Cooling)
					{
<<<<<<< HEAD
						// 保持当前方向
=======
						Moving.Direction = Directed.Direction;
>>>>>>> parent of 0f9a801 (Beta.2)
					}
				}
				else // 常规转向逻辑
				{
					float SlowFactor = 1;

					if (UNLIKELY(bIsFreezing))
					{
						// 减速转向
						const auto& Freezing = Subject.GetTraitRef<FFreezing, EParadigm::Unsafe>();
						SlowFactor = 1.0f - Freezing.FreezeStr;
					}

					float DeltaV = FMath::Abs(Moving.DesiredVelocity.Size2D() - Moving.CurrentVelocity.Size2D());

					if (AngleDegrees < 90.f && DeltaV > 100.f)//一致，朝向实际移动的方向
					{
<<<<<<< HEAD
						const FRotator CurrentRot = Directed.Direction.ToOrientationRotator();
						const FRotator DesiredRot = Moving.CurrentVelocity.GetSafeNormal2D().ToOrientationRotator();//the current velocity direction
						const FRotator InterpolatedRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, Move.TurnSpeed * SlowFactor);
=======
						case ERotationMode::RotationFollowVelocity:
						{
							Moving.Direction = DesiredDirection;
>>>>>>> parent of 0f9a801 (Beta.2)

						Directed.Direction = InterpolatedRot.Vector();
					}
					else//不一致，朝向想要移动的方向
					{
						const FRotator CurrentRot = Directed.Direction.ToOrientationRotator();
						const FRotator DesiredRot = DesiredMoveDirection.GetSafeNormal2D().ToOrientationRotator();
						const FRotator InterpolatedRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, Move.TurnSpeed * SlowFactor);

<<<<<<< HEAD
						Directed.Direction = InterpolatedRot.Vector();
						DesiredMoveDirection = Directed.Direction;
					}
				}

				// 减速
=======
							const FRotator InterpolatedRot = FMath::RInterpTo(
								CurrentRot,
								DesiredRot,
								DeltaTime,
								Move.RotationSpeed_Yaw * SlowFactor
							);

							Directed.Direction = InterpolatedRot.Vector();
							break;
						}

						case ERotationMode::VelocityFollowRotation:
						{
							// 将方向转换为Yaw角度进行插值
							const FRotator CurrentRot = Directed.Direction.ToOrientationRotator();
							const FRotator DesiredRot = DesiredDirection.ToOrientationRotator();

							// 使用角度插值（自动处理360度环绕）
							const FRotator InterpolatedRot = FMath::RInterpTo(
								CurrentRot,
								DesiredRot,
								DeltaTime,
								Move.RotationSpeed_Yaw * SlowFactor
							);

							// 将插值后的角度转换回方向向量
							Directed.Direction = InterpolatedRot.Vector();
							Moving.Direction = Directed.Direction;
							break;
						}
					}
				}

				//--------------------------- 速度计算 --------------------------//

				// 根据离目标距离不同加减速
				const float DistanceToGoal = FVector::Dist2D(AgentLocation, GoalLocation);

				if (LIKELY(DistanceToGoal > Attack.Range))
				{
					const TRange<float> InputRange(Move.SpeedDistRangeMap.Y, Move.SpeedDistRangeMap.W);
					const TRange<float> OutputRange(Move.SpeedDistRangeMap.X, Move.SpeedDistRangeMap.Z);

					DesiredSpeed *= FMath::GetMappedRangeValueClamped(InputRange, OutputRange, DistanceToGoal);
				}

>>>>>>> parent of 0f9a801 (Beta.2)
				if (UNLIKELY(bIsAttacking || bIsDying))
				{
					Moving.SpeedMult = 0.0f; // 攻击或死亡时停止
				}
				else if (UNLIKELY(bIsFreezing))
				{
					const auto& Freezing = Subject.GetTraitRef<FFreezing, EParadigm::Unsafe>();

					if (Freezing.FreezeTimeout > 0.0f)
					{
						Moving.SpeedMult *= (1.0f - Freezing.FreezeStr); //冰冻时减速
					}
				}

<<<<<<< HEAD
				// 速度-距离曲线
				float OtherRadius = Trace.TraceResult.HasTrait<FCollider>() ? Trace.TraceResult.GetTrait<FCollider>().Radius : 0;
				const float DistanceToGoal = FMath::Clamp(FVector::Dist2D(AgentLocation, GoalLocation) - OtherRadius, 0, FLT_MAX);
				const TRange<float> MoveInputRange(Move.MoveSpeedRangeMap.Y, Move.MoveSpeedRangeMap.W);
				const TRange<float> MoveOutputRange(Move.MoveSpeedRangeMap.X, Move.MoveSpeedRangeMap.Z);
				Moving.SpeedMult *= FMath::GetMappedRangeValueClamped(MoveInputRange, MoveOutputRange, DistanceToGoal);
=======
				// 计算当前方向与目标方向的夹角
				float Angle = FMath::Acos(FVector::DotProduct(Directed.Direction, DesiredDirection));
				float AngleDegrees = FMath::RadiansToDegrees(Angle);

				// 根据方向角调整速度，急转弯可以掉速度
				const TRange<float> InputRange(Move.SpeedTurnRangeMap.Y, Move.SpeedTurnRangeMap.W);
				const TRange<float> OutputRange(Move.SpeedTurnRangeMap.X, Move.SpeedTurnRangeMap.Z);

				DesiredSpeed *= FMath::GetMappedRangeValueClamped(InputRange, OutputRange, AngleDegrees);

				// 速度插值，平滑速度变化
				Moving.Speed = FMath::FInterpTo(Moving.Speed, DesiredSpeed, DeltaTime, (DesiredSpeed > Moving.Speed) ? Move.Acceleration : Move.BrakeDeceleration);
>>>>>>> parent of 0f9a801 (Beta.2)

				// 想要达到的理想速度
				FVector DesiredVelocity = Moving.SpeedMult * Move.MoveSpeed * DesiredMoveDirection;
				Moving.DesiredVelocity = FVector(DesiredVelocity.X, DesiredVelocity.Y, 0);

				//--------------------------- 物理效果 --------------------------//
				constexpr float BASE_TICK_TIME = 0.01667f; // 60Hz基准时间

<<<<<<< HEAD
				if (IsValid(Navigation.FlowField))// 没有流场则跳过，因为不知道地面高度，所以不考虑垂直运动
				{
					// 寻找最高地面
					bool bIsSet = false;
					FVector HighestGroundLocation = FVector::ZeroVector;
					FVector HighestGroundNormal = FVector::UpVector;

					// Center Point
					if (bInside_BaseFF)
					{
						bIsSet = true;
						HighestGroundLocation = Cell_BaseFF.worldLoc;
						HighestGroundNormal = Cell_BaseFF.normal;
					}

					// 8 Directions
					for (int32 i = 0; i < 8; ++i)
					{
						FVector BoundSamplePoint = Located.Location + FVector(Collider.Radius, 0, 0).RotateAngleAxis(i * 45.f, FVector::UpVector);

						bool bInside = false;
						FCellStruct Cell = FCellStruct{};
						Navigation.FlowField->GetCellAtLocation(BoundSamplePoint, bInside, Cell);

						if (bInside)
						{
							if (Cell.worldLoc.Z > HighestGroundLocation.Z)
							{
								bIsSet = true;
								HighestGroundLocation = Cell.worldLoc;
								HighestGroundNormal = Cell.normal;
							}
						}
					}

					//--------------------------- 运动状态 --------------------------//

					if (bIsSet)
					{
						// 计算投影高度
						const float PlaneD = -FVector::DotProduct(HighestGroundNormal, HighestGroundLocation);
						const float GroundHeight = (-PlaneD - HighestGroundNormal.X * AgentLocation.X - HighestGroundNormal.Y * AgentLocation.Y) / HighestGroundNormal.Z;
=======
				if (LIKELY(!Moving.bFalling))
				{
					Moving.Velocity = Moving.Direction * Moving.Speed;

					// 助推力衰减
					if (Moving.bLaunching)
					{
						constexpr float LAUNCH_DECAY_RATE = 0.9f;
						const float DecayFactor = FMath::Pow(LAUNCH_DECAY_RATE, DeltaTime / BASE_TICK_TIME);

						Moving.Velocity += Moving.LaunchForce;
						Moving.LaunchForce *= DecayFactor;

						if (Moving.LaunchForce.SizeSquared() < FMath::Square(100.0f))
						{
							Moving.bLaunching = false;;
							Moving.LaunchForce = FVector::ZeroVector;
						}
					}

					// 击退力衰减
					if (Moving.bKnockedBack)
					{
						constexpr float KNOCKBACK_DECAY_RATE = 0.9f;
						const float DecayFactor = FMath::Pow(KNOCKBACK_DECAY_RATE, DeltaTime / BASE_TICK_TIME);

						Moving.Velocity += FVector(Moving.KnockBackForce.X, Moving.KnockBackForce.Y, 0.0f);
						Moving.KnockBackForce *= DecayFactor;
>>>>>>> parent of 0f9a801 (Beta.2)

						if (Moving.KnockBackForce.SizeSquared() < FMath::Square(100.0f))
						{
<<<<<<< HEAD
							Moving.CurrentVelocity.Z += FMath::Clamp(Moving.FlyingHeight + GroundHeight - AgentLocation.Z, -100, 100);//fly at a certain height above ground
							Moving.CurrentVelocity.Z *= 0.9f;
						}
						else
						{
							const float CollisionThreshold = GroundHeight + Collider.Radius;

							// 高度状态判断
							if (AgentLocation.Z - CollisionThreshold > Collider.Radius * 0.1f)// need a bit of tolerance or it will be hard to decide is it is on ground or in the air
							{
								// 应用重力
								Moving.CurrentVelocity.Z += Move.Gravity * DeltaTime;

								// 进入/保持下落状态
								if (!Moving.bFalling)
								{
									Moving.bFalling = true;
								}
							}
							else
							{
								// 地面接触处理
								const float GroundContactThreshold = GroundHeight - Collider.Radius;

								// 着陆状态切换
								if (Moving.bFalling)
								{
									Moving.bFalling = false;
									Moving.CurrentVelocity = Moving.CurrentVelocity * Move.BounceVelocityDecay * FVector(1, 1, (FMath::Abs(Moving.CurrentVelocity.Z) > 100.f) ? -1 : 0);// zero out small number
								}

								// 平滑移动到地面
								AgentLocation.Z = FMath::FInterpTo(AgentLocation.Z, CollisionThreshold, DeltaTime, 25.0f);
							}
						}
					}
					else
					{
						if (Move.bCanFly)
						{
							Moving.CurrentVelocity.Z *= 0.9f;
						}
						else
						{
							// 应用重力
							Moving.CurrentVelocity.Z += Move.Gravity * DeltaTime;

							if (!Moving.bFalling)
							{
								Moving.bFalling = true;
							}
=======
							Moving.bKnockedBack = false;
							Moving.KnockBackForce = FVector::ZeroVector;
>>>>>>> parent of 0f9a801 (Beta.2)
						}
					}
				}
				else
<<<<<<< HEAD
				{
					Moving.CurrentVelocity.Z = 0;
				}

				//---------------------------- 物理效果 --------------------------//

				if(!Moving.bFalling && Moving.LaunchTimer > 0)// 地面摩擦力
				{
					FVector XYDir = Moving.CurrentVelocity.GetSafeNormal2D();
					float XYSpeed = Moving.CurrentVelocity.Size2D();
					XYSpeed = FMath::Clamp(XYSpeed - Move.Deceleration_Ground * DeltaTime, 0, XYSpeed);
					Moving.CurrentVelocity = XYDir * XYSpeed + FVector(0, 0, Moving.CurrentVelocity.Z);
					XYSpeed < 10.f ? Moving.LaunchTimer = 0 : Moving.LaunchTimer -= DeltaTime;
				}
				else if (Moving.bFalling && !Move.bCanFly)// 空气阻力
				{
					FVector XYDir = Moving.CurrentVelocity.GetSafeNormal();
					float XYSpeed = Moving.CurrentVelocity.Size();
					XYSpeed = FMath::Clamp(XYSpeed - Move.Deceleration_Air * DeltaTime, 0, XYSpeed);
					Moving.CurrentVelocity = XYDir * XYSpeed;
=======
				{
					// 空气阻力
					constexpr float AIR_DECAY_RATE = 0.99f;
					const float DecayFactor = FMath::Pow(AIR_DECAY_RATE, DeltaTime / BASE_TICK_TIME);
					Moving.Velocity.X *= DecayFactor;
					Moving.Velocity.Y *= DecayFactor;
				}

				//--------------------------- 动画控制 --------------------------//

				float Speed = (Located.Location - Located.preLocation).Size2D() / DeltaTime;

				if (LIKELY(!bIsAttacking && !bIsDying))
				{
					Animation.SubjectState = (Speed > 10.f) ? ESubjectState::Moving : ESubjectState::Idle;// switch between idle and move based on speed(ignore small value)
				}
				else if (bIsDying && Speed < 100.f && !Subject.HasTrait<FStatic>())
				{
					Subject.SetTraitDeferred(FStatic{});
>>>>>>> parent of 0f9a801 (Beta.2)
				}

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 碰撞与避障
	#pragma region
	{
<<<<<<< HEAD
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Avoidance");// agent only do avoidance in xy, not z
=======
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("RVO2");
>>>>>>> parent of 0f9a801 (Beta.2)
		NeighborGrid->Evaluate();
	}
	#pragma endregion

	// 垂直运动
	#pragma region
	{
<<<<<<< HEAD
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("IdleToMoveAnim");

		// 初始化过滤器
		FFilter Filter = FFilter::Make<FAgent, FAnimation, FMoving, FDeath>();
=======
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentZMovement");

		// 初始化过滤器
		FFilter Filter = FFilter::Make<FAgent, FRendering, FMove, FMoving, FDirected, FLocated, FCollider, FNavigation>();
>>>>>>> parent of 0f9a801 (Beta.2)
		Filter.Exclude<FAppearing>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
<<<<<<< HEAD
				FAnimation& Animation,
				FMoving& Moving,
				FDeath& Death)
=======
				FMove& Move,
				FMoving& Moving,
				FDirected& Directed,
				FLocated& Located,
				FCollider& Collider,
				FNavigation& Navigation)
>>>>>>> parent of 0f9a801 (Beta.2)
			{
				if (!Move.bEnable) return;

				//--------------------------- 基础检查 --------------------------//
				
				// 死亡区域检测
				if (Located.Location.Z < Move.KillZ)
				{
					Subject.DespawnDeferred();
					return;
				}

				// 飞行单位跳过处理
				if (Move.bCanFly) return;

				//--------------------------- 流场数据准备 ------------------------//

				if (!IsValid(Navigation.FlowField))
				{
<<<<<<< HEAD
					Animation.SubjectState = (RealSpeed2D > Animation.UseMoveAnimAboveSpeed) ? ESubjectState::Moving : ESubjectState::Idle;// switch between idle and move based on speed(ignore small value)
				}
				else if (bIsDying && RealSpeed2D < 100.f && Death.bDisableCollision && !Subject.HasTrait<FCorpse>())
=======
					if (Navigation.FlowFieldActor)
					{
						Navigation.FlowField = Navigation.FlowFieldActor.LoadSynchronous();
						return;
					}
					else
					{
						return;
					}
				}

				// 计算流场基准位置
				const FVector FlowFieldOrigin = Navigation.FlowField->GetActorLocation() - FVector(Navigation.FlowField->flowFieldSize.X / 2, Navigation.FlowField->flowFieldSize.Y / 2, 0);

				// 碰撞参数
				const float CellSize = Navigation.FlowField->cellSize;
				const float ColliderRadius = Collider.Radius;
				const FVector CurrentLocation = Located.Location;

				//--------------------------- 地面检测 --------------------------//

				// 计算检测范围
				const FVector LocalOffset = CurrentLocation - FlowFieldOrigin;

				const int32 MinX = FMath::Clamp(FMath::RoundToInt((LocalOffset.X - CellSize / 2 - ColliderRadius) / CellSize), 0, Navigation.FlowField->xNum - 1);
				const int32 MaxX = FMath::Clamp(FMath::RoundToInt((LocalOffset.X - CellSize / 2 + ColliderRadius) / CellSize), 0, Navigation.FlowField->xNum - 1);
				const int32 MinY = FMath::Clamp(FMath::RoundToInt((LocalOffset.Y - CellSize / 2 - ColliderRadius) / CellSize), 0, Navigation.FlowField->yNum - 1);
				const int32 MaxY = FMath::Clamp(FMath::RoundToInt((LocalOffset.Y - CellSize / 2 + ColliderRadius) / CellSize), 0, Navigation.FlowField->yNum - 1);

				// 寻找最高地面
				TOptional<FVector> HighestGroundLocation;
				FVector HighestGroundNormal = FVector::UpVector;

				for (int32 X = MinX; X <= MaxX; ++X)
>>>>>>> parent of 0f9a801 (Beta.2)
				{
					for (int32 Y = MinY; Y <= MaxY; ++Y)
					{
						const FCellStruct& Cell = Navigation.FlowField->InitialCellsMap[FVector2D(X, Y)];
						if (!HighestGroundLocation.IsSet() || Cell.worldLoc.Z > HighestGroundLocation->Z)
						{
							HighestGroundLocation = Cell.worldLoc;
							HighestGroundNormal = Cell.normal;
						}
					}
				}

				//--------------------------- 运动计算 --------------------------//

				const FVector Velocity = (CurrentLocation - Located.preLocation) / DeltaTime;
				FVector NewLocation = CurrentLocation;

				if (HighestGroundLocation.IsSet())
				{
					const FVector& GroundLocation = HighestGroundLocation.GetValue();
					const FVector& GroundNormal = HighestGroundNormal;

					// 计算投影高度
					const FVector ProjectionPlanePoint(GroundLocation.X, GroundLocation.Y, 0);
					const float PlaneD = -FVector::DotProduct(GroundNormal, ProjectionPlanePoint);
					const float GroundHeight = (-PlaneD - GroundNormal.X * CurrentLocation.X - GroundNormal.Y * CurrentLocation.Y) / GroundNormal.Z;
					const float CollisionThreshold = GroundHeight + ColliderRadius;

					// 高度状态判断
					if (CurrentLocation.Z > CollisionThreshold)
					{
						// 进入/保持下落状态
						if (!Moving.bFalling)
						{
							Moving.bFalling = true;
							Moving.Velocity = Velocity;
						}

						// 应用重力
						Moving.Velocity.Z += Move.Gravity * DeltaTime;
						NewLocation.Z += Moving.Velocity.Z * DeltaTime;
					}
					else
					{
						// 地面接触处理
						const float GroundContactThreshold = GroundHeight - ColliderRadius;
						const bool bIsInContactRange = CurrentLocation.Z > GroundContactThreshold;

						if (bIsInContactRange)
						{
							// 平滑着陆
							NewLocation.Z = FMath::FInterpTo(CurrentLocation.Z, CollisionThreshold, DeltaTime, 35.0f);

							// 着陆状态切换
							if (Moving.bFalling)
							{
								Moving.bFalling = false;
								Moving.Velocity.Z = 0;
							}
						}
						else
						{
							Subject.DespawnDeferred();
						}
					}
				}
				else
				{
					// 无地面参照时的自由下落
					if (!Moving.bFalling)
					{
						Moving.bFalling = true;
						Moving.Velocity = Velocity;
					}

					Moving.Velocity.Z += Move.Gravity * DeltaTime;
					NewLocation.Z += Moving.Velocity.Z * DeltaTime;
				}

				// 更新最终位置
				Located.Location = NewLocation;

			}, ThreadsCount, BatchSize);

		Mechanism->ApplyDeferreds();
	}
	#pragma endregion

	//--------------------------其它---------------------------

	// 播放音效
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("PlaySound");

		for (int32 i = 0; i < NumSoundsPerFrame; ++i)
		{
			if (SoundsToPlay.IsEmpty())
			{
				break;
			}

			TSoftObjectPtr<USoundBase> Sound;

			if (!SoundsToPlay.Dequeue(Sound))
			{
				break;
			}

			// 异步加载音效并绑定回调函数以在加载完成后播放音效
			StreamableManager.RequestAsyncLoad(Sound.ToSoftObjectPath(), FStreamableDelegate::CreateLambda([this, Sound]()
				{
					// 播放加载完成的音效
					UGameplayStatics::PlaySound2D(GetWorld(), Sound.Get(), SoundVolume);
				}));
		}
	}
	#pragma endregion

	// Spawn Actors
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Spawn Actors");

		FFilter Filter = FFilter::Make<FSpawningActor>();

		Mechanism->Operate<FUnsafeChain>(Filter,
			[&](FSubjectHandle Subject,
				FSpawningActor& SpawningActor)
			{
				if (SpawningActor.Quantity > 0 && SpawningActor.SpawnClass)
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

					for (int32 i = 0; i < SpawningActor.Quantity; ++i)
					{
						AActor* Actor = CurrentWorld->SpawnActor<AActor>(SpawningActor.SpawnClass, SpawningActor.Trans, SpawnParams);

						if (Actor != nullptr)
						{
							Actor->SetActorScale3D(SpawningActor.Trans.GetScale3D());
						}
					}
				}

				Subject.Despawn();
			});
	}
	#pragma endregion

	//------------------------更新渲染------------------------

	// 动画状态机
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentStateMachine");
		static const auto Filter = FFilter::Make<FAgent, FAnimation, FRendering, FAppear, FAttack, FDeath>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FAnimation& Anim,
				FAppear& Appear,
				FAttack& Attack,
				FDeath& Death)
			{
				if (Anim.SubjectState != Anim.PreviousSubjectState && Anim.AnimLerp == 1)
				{
					switch (Anim.SubjectState)
					{
						default:
						{
							CopyAnimData(Anim);
							Anim.AnimIndex1 = Anim.IndexOfIdleAnim;
							Anim.AnimCurrentTime1 = GetGameTimeSinceCreation();
							Anim.AnimOffsetTime1 = 0;
							Anim.AnimPauseTime1 = 0;
							Anim.AnimPlayRate1 = 1;
							break;
						}

						case ESubjectState::None:
						{
							CopyAnimData(Anim);
							Anim.AnimIndex1 = Anim.IndexOfIdleAnim;
							Anim.AnimCurrentTime1 = GetGameTimeSinceCreation();
							Anim.AnimOffsetTime1 = 0;
							Anim.AnimPauseTime1 = 0;
							Anim.AnimPlayRate1 = 0;
							break;
						}

						case ESubjectState::Appearing:
						{
							Anim.AnimIndex1 = Anim.IndexOfAppearAnim;
							Anim.AnimCurrentTime1 = GetGameTimeSinceCreation();
							Anim.AnimOffsetTime1 = 0;
							Anim.AnimPauseTime1 = Anim.AppearAnimLength;
							Anim.AnimPlayRate1 = Anim.AppearAnimLength / Appear.Duration;
							Anim.AnimLerp = 1;// since appearing is definitely the first anim to play
							Anim.Dissolve = 0;
							break;
						}

						case ESubjectState::Idle:
						{
							CopyAnimData(Anim);

							Anim.AnimIndex1 = Anim.IndexOfIdleAnim;
							Anim.AnimCurrentTime1 = GetGameTimeSinceCreation();
							Anim.AnimOffsetTime1 = 0;
							Anim.AnimPauseTime1 = 0;

							if (Subject.HasTrait<FFreezing>())
							{
								Anim.AnimPlayRate1 = 1 - Subject.GetTraitRef<FFreezing, EParadigm::Unsafe>().FreezeStr;
							}
							else
							{
								Anim.AnimPlayRate1 = 1;
							}
							break;
						}

						case ESubjectState::Moving:
						{
							CopyAnimData(Anim);
							Anim.AnimIndex1 = Anim.IndexOfMoveAnim;
							Anim.AnimCurrentTime1 = GetGameTimeSinceCreation();
							Anim.AnimOffsetTime1 = 0;
							Anim.AnimPauseTime1 = 0;

							if (Subject.HasTrait<FFreezing>())
							{
								Anim.AnimPlayRate1 = 1 - Subject.GetTraitRef<FFreezing, EParadigm::Unsafe>().FreezeStr;
							}
							else
							{
								Anim.AnimPlayRate1 = 1;
							}
							break;
						}

						case ESubjectState::Attacking:
						{
							CopyAnimData(Anim);
							Anim.AnimIndex1 = Anim.IndexOfAttackAnim;
							Anim.AnimCurrentTime1 = GetGameTimeSinceCreation();
							Anim.AnimOffsetTime1 = 0;
							Anim.AnimPauseTime1 = Anim.AttackAnimLength;
							Anim.AnimPlayRate1 = Anim.AttackAnimLength / Attack.DurationPerRound;
							break;
						}

						case ESubjectState::Dying:
						{
							CopyAnimData(Anim);
							Anim.AnimIndex1 = Anim.IndexOfDeathAnim;
							Anim.AnimCurrentTime1 = GetGameTimeSinceCreation();
							Anim.AnimOffsetTime1 = 0;
							Anim.AnimPauseTime1 = Anim.DeathAnimLength;
							Anim.AnimPlayRate1 = 1;
							break;
						}
					}
					Anim.PreviousSubjectState = Anim.SubjectState;
				}
				Anim.AnimLerp = FMath::Clamp(Anim.AnimLerp + DeltaTime * Anim.LerpSpeed, 0, 1);

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 重置渲染数据
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("ClearValidTransforms");

		FFilter Filter = FFilter::Make<FRenderBatchData>().Exclude<FDying>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FRenderBatchData& Data)
			{
				Data.ValidTransforms.Reset();

				Data.Text_Location_Array.Reset();
				Data.Text_Value_Style_Scale_Offset_Array.Reset();

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 合批渲染数据
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentRender");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FDirected, FScaled, FLocated, FAnimation, FHealth, FHealthBar, FCollider>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FRendering& Rendering,
				FDirected& Directed,
				FScaled& Scaled,
				FLocated& Located,
				FAnimation& Anim,
				FHealth& Health,
				FHealthBar& HealthBar,
				FCollider& Collider)
			{
				if (!Rendering.Renderer.IsValid())
				{
					// 标记为死亡
					Subject.SetTraitDeferred(FDying{});
					return;
				}

				FRenderBatchData& Data = Rendering.Renderer.GetTraitRef<FRenderBatchData, EParadigm::Unsafe>();

				FQuat Rotation{ FQuat::Identity };
				Rotation = Directed.Direction.Rotation().Quaternion();

				FVector FinalScale(Data.Scale);
				FinalScale *= Scaled.renderFactors;

				float Radius = Collider.Radius;

				// 在计算转换时减去Radius
				FTransform SubjectTransform( Rotation * Data.OffsetRotation.Quaternion(),Located.Location + Data.OffsetLocation - FVector(0, 0, Radius), FinalScale); // 减去Z轴上的Radius					

				int32 InstanceId = Rendering.InstanceId;

				Data.Lock();

				Data.ValidTransforms[InstanceId] = true;
				Data.Transforms[InstanceId] = SubjectTransform;

				// Transforms
				Data.LocationArray[InstanceId] = SubjectTransform.GetLocation();
				Data.OrientationArray[InstanceId] = SubjectTransform.GetRotation();
				Data.ScaleArray[InstanceId] = SubjectTransform.GetScale3D();

				// Dynamic params 0
				Data.Anim_Index0_Index1_PauseTime0_PauseTime1_Array[InstanceId] = FVector4(Anim.AnimIndex0,Anim.AnimIndex1,Anim.AnimPauseTime0,Anim.AnimPauseTime1);

				// Dynamic params 1
				Data.Anim_TimeStamp0_TimeStamp1_PlayRate0_Playrate1_Array[InstanceId] = FVector4(Anim.AnimCurrentTime0 + Anim.AnimOffsetTime0, Anim.AnimCurrentTime1 + Anim.AnimOffsetTime1, Anim.AnimPlayRate0, Anim.AnimPlayRate1);

				// Pariticle color R
				Data.Anim_Lerp_Array[InstanceId] = Anim.AnimLerp;

				// Dynamic params 2
				Data.Mat_HitGlow_Freeze_Burn_Dissolve_Array[InstanceId] = FVector4(Anim.HitGlow, Anim.FreezeFx, Anim.BurnFx, Anim.Dissolve);

				// HealthBar
				Data.HealthBar_Opacity_CurrentRatio_TargetRatio_Array[InstanceId] = FVector(HealthBar.Opacity, HealthBar.CurrentRatio, HealthBar.TargetRatio);

				Data.Unlock();

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	#pragma region
	{
		FFilter Filter = FFilter::Make<FAgent, FRendering, FPoppingText>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FRendering Rendering,
				FPoppingText PoppingText)
			{
				FRenderBatchData& Data = Rendering.Renderer.GetTraitRef<FRenderBatchData, EParadigm::Unsafe>();

				TArray<FVector> LocationArray = PoppingText.TextLocationArray;
				TArray<FVector4> CombinedArray = PoppingText.Text_Value_Style_Scale_Offset_Array;

				Data.Lock();
				Data.Text_Location_Array.Append(LocationArray);
				Data.Text_Value_Style_Scale_Offset_Array.Append(CombinedArray);
				Data.Unlock();

				Subject.RemoveTraitDeferred<FPoppingText>();

			}, ThreadsCount, BatchSize);

		Mechanism->ApplyDeferreds();
	}
	#pragma endregion

	// Write Pooling Info
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Write Pooling Info");

		FFilter Filter = FFilter::Make<FRenderBatchData>().Exclude<FDying>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(),MaxThreadsAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FRenderBatchData& Data)
			{
				// 重置和隐藏限制数组成员
				Data.FreeTransforms.Reset();

				for (int32 i = Data.ValidTransforms.IndexOf(false);
					i < Data.Transforms.Num();
					i = Data.ValidTransforms.IndexOf(false, i + 1))
				{
					Data.FreeTransforms.Add(i);
					Data.InsidePool_Array[i] = true;
				}

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 同步至Niagara
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("SendDataToNiagara");

		FFilter Filter = FFilter::Make<FRenderBatchData>().Exclude<FDying>();

		Mechanism->Operate<FUnsafeChain>(Filter,
			[&](FSubjectHandle Subject,
				FRenderBatchData& Data)
			{
				// ------------------Transform---------------------------------

				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
					Data.SpawnedNiagaraSystem,
					FName("LocationArray"),
					Data.LocationArray
				);

				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayQuat(
					Data.SpawnedNiagaraSystem,
					FName("OrientationArray"),
					Data.OrientationArray
				);

				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
					Data.SpawnedNiagaraSystem,
					FName("ScaleArray"),
					Data.ScaleArray
				);

				// -----------------VAT Auto Play------------------------------

				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
					Data.SpawnedNiagaraSystem,
					FName("Anim_Index0_Index1_PauseTime0_PauseTime1_Array"),
					Data.Anim_Index0_Index1_PauseTime0_PauseTime1_Array
				);

				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
					Data.SpawnedNiagaraSystem,
					FName("Anim_TimeStamp0_TimeStamp1_PlayRate0_Playrate1_Array"),
					Data.Anim_TimeStamp0_TimeStamp1_PlayRate0_Playrate1_Array
				);

				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(
					Data.SpawnedNiagaraSystem,
					FName("Anim_Lerp_Array"),
					Data.Anim_Lerp_Array
				);

				// ------------------Material FX---------------------------

				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
					Data.SpawnedNiagaraSystem,
					FName("Mat_HitGlow_Freeze_Burn_Dissolve_Array"),
					Data.Mat_HitGlow_Freeze_Burn_Dissolve_Array
				);

				// ------------------HealthBar---------------------------------

				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
					Data.SpawnedNiagaraSystem,
					FName("HealthBar_Opacity_CurrentRatio_TargetRatio_Array"),
					Data.HealthBar_Opacity_CurrentRatio_TargetRatio_Array
				);

				// ------------------Pop Text----------------------------------

				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
					Data.SpawnedNiagaraSystem,
					FName("Text_Location_Array"),
					Data.Text_Location_Array
				);

				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
					Data.SpawnedNiagaraSystem,
					FName("Text_Value_Style_Scale_Offset_Array"),
					Data.Text_Value_Style_Scale_Offset_Array
				);
				
				// ------------------Others------------------------------------

				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayBool(
					Data.SpawnedNiagaraSystem,
					FName("InsidePool_Array"),
					Data.InsidePool_Array
				);
			});
	}
	#pragma endregion

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FResult ABattleFrameGameMode::ApplyDamageToSubjects(const TArray<FSubjectHandle> Subjects, const TArray<FSubjectHandle> IgnoreSubjects, FSubjectHandle DmgInstigator, FVector HitFromLocation, const FDmgSphere DmgSphere, const FDebuff Debuff)
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("ApplyDamageToSubjects");

	// Record for deferred spawning of TemporalDamager
	FTemporalDamaging TemporalDamaging;

	// 添加一个数组来存储唯一的敌人句柄
	TSet<FSubjectHandle> UniqueHandles;

	FResult Result;

	for (const auto& Overlapper : Subjects)
	{
		if (IgnoreSubjects.Contains(Overlapper)) continue;

		int32 previousNum = UniqueHandles.Num();
		UniqueHandles.Add(Overlapper);

		if (UniqueHandles.Num() == previousNum) continue;

		if (Overlapper.HasTrait<FHealth>())
		{
			bool bHasLocated = Overlapper.HasTrait<FLocated>();

			//-------------伤害和抗性------------

			float NormalDmgMult = 1;
			float KineticDmgMult = 1;
			float KineticDebuffMult = 1;
			float FireDmgMult = 1;
			float FireDebuffMult = 1;
			float IceDmgMult = 1;
			float IceDebuffMult = 1;
			float PercentDmgMult = 1;

			// 抗性 如果有的话
			if (Overlapper.HasTrait<FDefence>())
			{
				const auto& Defence = Overlapper.GetTraitRef<FDefence, EParadigm::Unsafe>();

				NormalDmgMult = 1 - Defence.NormalDmgImmune;
				KineticDmgMult = 1 - Defence.KineticDmgImmune;
				KineticDebuffMult = 1 - Defence.KineticDebuffImmune;
				FireDmgMult = 1 - Defence.FireDmgImmune;
				FireDebuffMult = 1 - Defence.FireDebuffImmune;
				IceDmgMult = 1 - Defence.IceDmgImmune;
				IceDebuffMult = 1 - Defence.IceDebuffImmune;
				PercentDmgMult = 1.f - Defence.PercentDmgImmune;
			}

			float NormalDamage = DmgSphere.Damage * NormalDmgMult;
			float KineticDamage = DmgSphere.KineticDmg * KineticDmgMult;
			float FireDamage = DmgSphere.FireDmg * FireDmgMult;
			float IceDamage = DmgSphere.IceDmg * IceDmgMult;

			// 总基础伤害
			float BaseDamage = NormalDamage + KineticDamage + FireDamage + IceDamage;

			// 百分比伤害
			auto& Health = Overlapper.GetTraitRef<FHealth, EParadigm::Unsafe>();
			float PercentageDamage = Health.Maximum * DmgSphere.PercentDmg * PercentDmgMult;

			// 总伤害
			float CombinedDamage = BaseDamage + PercentageDamage;

			// 考虑暴击后伤害
			auto [bIsCrit, PostCritDamage] = ProcessCritDamage(CombinedDamage, DmgSphere.CritMult, DmgSphere.CritProbability);

			// 限制伤害以不大于剩余血量
			float ClampedDamage = FMath::Min(PostCritDamage, Health.Current);

			Result.DamagedSubjects.Add(Overlapper);
			Result.IsCritical.Add(bIsCrit);
			Result.IsKill.Add(Health.Current == ClampedDamage);
			Result.DmgDealt.Add(ClampedDamage);

			// 应用伤害
			Health.DamageToTake.Enqueue(ClampedDamage);

			// 记录伤害施加者
			if (DmgInstigator.IsValid())
			{
				Health.DamageInstigator.Enqueue(DmgInstigator);
			}
			else
			{
				Health.DamageInstigator.Enqueue(FSubjectHandle());
			}

			// ------------生成文字--------------

			if (Overlapper.HasTrait<FTextPopUp>() && bHasLocated)
			{
				const auto TextPopUp = Overlapper.GetTrait<FTextPopUp>();

				if (TextPopUp.Enable)
				{
					float Style = 0;
					float Radius = 0.f;
					FVector Location = Overlapper.GetTrait<FLocated>().Location;

					if (!bIsCrit)
					{
						if (PostCritDamage < TextPopUp.WhiteTextBelowPercent)
						{
							Style = 0;
						}
						else if (PostCritDamage < TextPopUp.OrangeTextAbovePercent)
						{
							Style = 1;
						}
						else
						{
							Style = 2;
						}
					}
					else
					{
						Style = 3;
					}

					if (Overlapper.HasTrait<FCollider>())
					{
						Radius = Overlapper.GetTrait<FCollider>().Radius;
					}

					Location += FVector(0, 0, Radius);

					QueueText(Overlapper, PostCritDamage, Style, TextPopUp.TextScale, Radius * 1.1, Location);
				}
			}

			//--------------Debuff--------------

			FVector HitDirection = FVector::ZeroVector;

			if (bHasLocated)
			{
				HitDirection = (Overlapper.GetTrait<FLocated>().Location - HitFromLocation).GetSafeNormal2D();
			}

			// 击退
			if (Debuff.bCanKnockback)
			{
				if (Overlapper.HasTrait<FMove>() && Overlapper.HasTrait<FMoving>())
				{
					auto& Moving = Overlapper.GetTraitRef<FMoving, EParadigm::Unsafe>();
					const auto& Move = Overlapper.GetTraitRef<FMove, EParadigm::Unsafe>();
					FVector KnockbackForce = Debuff.KnockbackSpeed * HitDirection * KineticDebuffMult;
					FVector CombinedForce = Moving.KnockBackForce + KnockbackForce;
					FVector CombinedDirection = CombinedForce.GetSafeNormal2D();
					float CombinedSize = FMath::Clamp(CombinedForce.Size2D(), 0, Move.MaxImpulse);

					Moving.Lock();
					Moving.KnockBackForce = CombinedDirection * CombinedSize; // 累加击退力
					Moving.Unlock();

					if (Moving.KnockBackForce.Size2D() > 100.f)
					{
						Moving.bKnockedBack = true;
					}
				}
			}

			// 冰冻
			if (Debuff.bCanFreeze)
			{
				if (Overlapper.HasTrait<FFreezing>())
				{
					auto& CurrentFreezing = Overlapper.GetTraitRef<FFreezing, EParadigm::Unsafe>();

					if (Debuff.FreezeTime > CurrentFreezing.FreezeTimeout)
					{
						CurrentFreezing.FreezeTimeout = Debuff.FreezeTime;
					}
				}
				else
				{
					FFreezing NewFreezing;
					NewFreezing.FreezeTimeout = Debuff.FreezeTime * IceDebuffMult;
					NewFreezing.FreezeStr = Debuff.FreezeStr * IceDebuffMult;

					if (NewFreezing.FreezeTimeout > 0 && NewFreezing.FreezeStr > 0)
					{
						if (Overlapper.HasTrait<FAnimation>())
						{
							auto& Animation = Overlapper.GetTraitRef<FAnimation, EParadigm::Unsafe>();
							Animation.PreviousSubjectState = ESubjectState::Dirty; // 强制刷新动画状态机
							Animation.FreezeFx = 1;
						}
						Overlapper.SetTraitDeferred(NewFreezing);
					}
				}
			}

			// 灼烧
			if (Debuff.bCanBurn)
			{
				TemporalDamaging.TotalTemporalDamage = BaseDamage * Debuff.BurnDmgRatio * FireDebuffMult;

				if (TemporalDamaging.TotalTemporalDamage > 0)
				{
					TemporalDamaging.RemainingTemporalDamage = TemporalDamaging.TotalTemporalDamage;

					if (DmgInstigator.IsValid())
					{
						TemporalDamaging.TemporalDamageInstigator = DmgInstigator;
					}
					else
					{
						TemporalDamaging.TemporalDamageInstigator = FSubjectHandle();
					}

					TemporalDamaging.TemporalDamageTarget = Overlapper;

					Mechanism->SpawnSubject(TemporalDamaging);
				}
			}

			//-----------其它效果------------

			if (Overlapper.HasTrait<FHit>())
			{
				const auto& Hit = Overlapper.GetTraitRef<FHit, EParadigm::Unsafe>();

				// Glow
				if (Hit.bCanGlow)
				{
					Overlapper.ObtainTraitDeferred<FHitGlow>();// we not glowing we add the trait. if is glowing, do nothing.
				}

				// Jiggle
				if (Hit.SqueezeSquashStr != 0.f)
				{
					Overlapper.ObtainTraitDeferred<FSqueezeSquash>();
				}

				// Fx
				if (Overlapper.HasTrait<FFX>())
				{
					const auto& FX = Overlapper.GetTraitRef<FFX, EParadigm::Unsafe>();

					if (Hit.bCanSpawnFx && FX.HitFx.SubType != ESubType::None)
					{
						FRotator CombinedRotator = (FQuat(FX.HitFx.Transform.GetRotation()) * FQuat(HitDirection.Rotation())).Rotator();
						QueueFx(FSubjectHandle{ Overlapper }, FTransform(CombinedRotator, FX.HitFx.Transform.GetLocation(), FX.HitFx.Transform.GetScale3D()), FX.HitFx.SubType);
					}
				}

				// Sound
				if (Overlapper.HasTrait<FSound>())
				{
					const auto& Sound = Overlapper.GetTraitRef<FSound, EParadigm::Unsafe>();

					if (Hit.bCanPlaySound && Sound.HitSound)
					{
						QueueSound(Sound.HitSound);
					}
				}
			}
		}
	}

	return Result;
}

// 计算实际伤害，并返回一个pair，第一个元素是是否暴击，第二个元素是实际伤害
FORCEINLINE std::pair<bool, float> ABattleFrameGameMode::ProcessCritDamage(float BaseDamage, float damageMult, float Probability)
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

// 生成音效播放Subject
FORCEINLINE void ABattleFrameGameMode::QueueSound(TSoftObjectPtr<USoundBase> Sound)
{
	float RandomValue = FMath::FRand();
	SoundsToPlay.Enqueue(Sound);
}

// 把受击数字添加到播放序列
FORCEINLINE void ABattleFrameGameMode::QueueText(FSubjectHandle Subject, float Value, float Style, float Scale, float Radius, FVector Location)
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

FORCEINLINE void ABattleFrameGameMode::QueueFx(FSubjectHandle Subject, FTransform Transfrom, ESubType SubType)
{
	// 将偏移向量转换到Subject的本地坐标系
	FLocated Located = Subject.GetTrait<FLocated>();
	FRotator ForwardRotation = Subject.GetTrait<FDirected>().Direction.Rotation();
	FVector LocationOffsetLocal = ForwardRotation.RotateVector(Transfrom.GetLocation());

	FLocated FxLocated = { Located.Location + LocationOffsetLocal };  // 应用转换后的偏移量
	FDirected FxDirected = { Transfrom.GetRotation().GetForwardVector()};
	FScaled FxScaled = { Transfrom.GetScale3D() };

	FSubjectRecord FxRecord;
	FxRecord.SetTrait(FSpawningFx{});  // Set the SpawningFx trait
	FxRecord.SetTrait(FxLocated);
	FxRecord.SetTrait(FxDirected);
	FxRecord.SetTrait(FxScaled);

	// Use the function to set the appropriate FSubTypeX based on attackFxSubType
	UBattleFrameFunctionLibraryRT::SetSubTypeTraitByEnum(SubType, FxRecord);

	GetMechanism()->SpawnSubjectDeferred(FxRecord);
}

FORCEINLINE void ABattleFrameGameMode::CopyAnimData(FAnimation& Animation)
{
	Animation.AnimLerp = 0;

	Animation.AnimIndex0 = Animation.AnimIndex1;
	Animation.AnimCurrentTime0 = Animation.AnimCurrentTime1;
	Animation.AnimOffsetTime0 = Animation.AnimOffsetTime1;
	Animation.AnimPauseTime0 = Animation.AnimPauseTime1;
	Animation.AnimPlayRate0 = Animation.AnimPlayRate1;
}