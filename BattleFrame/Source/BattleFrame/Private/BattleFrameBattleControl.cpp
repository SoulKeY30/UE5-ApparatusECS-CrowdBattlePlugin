/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#include "BattleFrameBattleControl.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/ThreadManager.h"
#include "EngineUtils.h"

// Niagara 插件
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

// Apparatus 插件
#include "SubjectiveActorComponent.h"

// FlowFieldCanvas 插件
#include "FlowField.h"

// BattleFrame 插件
#include "NeighborGridActor.h"
#include "NeighborGridComponent.h"


ABattleFrameBattleControl* ABattleFrameBattleControl::Instance = nullptr;

void ABattleFrameBattleControl::BeginPlay()
{
	Super::BeginPlay();

	Instance = this;

	DefineFilters();
}

void ABattleFrameBattleControl::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("BattleControlTick");

	Super::Tick(DeltaTime);

	CurrentWorld = GetWorld();

	if (UNLIKELY(CurrentWorld))
	{
		Mechanism = UMachine::ObtainMechanism(CurrentWorld);

		NeighborGrids.Reset(); // to do : add in multiple grid support

		for (TActorIterator<ANeighborGridActor> It(CurrentWorld); It; ++It)
		{
			NeighborGrids.Add(It->GetComponent());
		}

		if (UNLIKELY(!bIsFilterReady))
		{
			DefineFilters();
		}
	}

	if (UNLIKELY(bGamePaused || !CurrentWorld || !Mechanism || NeighborGrids.IsEmpty())) return;

	float SafeDeltaTime = FMath::Clamp(DeltaTime, 0, 0.0333f);

	// 更新碰撞网格 | Update NeighborGrid
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Update NeighborGrid");

		for (UNeighborGridComponent* Grid : NeighborGrids)
		{
			if (Grid)
			{
				Grid->Update();
			}
		}
	}
	#pragma endregion

	//--------------------数据统计 | Statistics----------------------

	// 统计Agent数量 | Agent Counter
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Agent Count");

		auto Chain = Mechanism->Enchain(AgentCountFilter);
		AgentCount = Chain->IterableNum();
		Chain->Reset(true);
	}
	#pragma endregion

	// 统计游戏时长 | Game Time Counter
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Agent Age");

		auto Chain = Mechanism->EnchainSolid(AgentAgeFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently([&](FStatistics& Stats)
		{
			if (Stats.bEnable)
			{
				Stats.totalTime += SafeDeltaTime;
			}

		}, ThreadsCount, BatchSize);
	}
	#pragma endregion


	//----------------------出生 | Appear-----------------------

	// 出生总 | Appear Main
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentAppearMain");

		auto Chain = Mechanism->EnchainSolid(AgentAppeaFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FLocated& Located,
				FDirected& Directed,
				FAppear& Appear,
				FAppearing& Appearing,
				FAnimation& Animation)
			{
				// Initial execute
				if (Appearing.time == 0)
				{
					// Actor
					for (FActorSpawnConfig Config : Appear.SpawnActor)
					{
						Config.SubjectTrans = FTransform(Directed.Direction.Rotation(), Located.Location, Config.Transform.GetScale3D());
						QueueActor(Config);
					}

					// Fx
					for (FFxConfig Config : Appear.SpawnFx)
					{
						Config.SubjectTrans = FTransform(Directed.Direction.Rotation(),Located.Location,Config.Transform.GetScale3D());
						QueueFx(Config);
					}

					// Sound
					for (FSoundConfig Config : Appear.PlaySound)
					{
						Config.SubjectTrans = FTransform(Directed.Direction.Rotation(),Located.Location,Config.Transform.GetScale3D());
						QueueSound(Config);
					}
				}
				
				if (Appearing.time >= Appear.Delay)
				{
					if (!Appear.bAppearStarted)
					{
						Appear.bAppearStarted = true;

						// Dissolve In
						if (Appear.bCanDissolveIn)
						{
							Subject.SetTraitDeferred(FAppearDissolve{});
						}
						else
						{
							Animation.Dissolve = 0;// unhide
						}

						// Animation
						if (Appear.bCanPlayAnim)
						{
							Subject.SetTraitDeferred(FAppearAnim{});
						}
					}
				}

				if (Appearing.time >= (Appear.Duration + Appear.Delay))
				{
					Subject.RemoveTraitDeferred<FAppearing>();
				}

				Appearing.time += SafeDeltaTime;

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 出生动画 | Birth Anim
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentAppearAnim");

		auto Chain = Mechanism->EnchainSolid(AgentAppearAnimFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FAnimation& Animation,
				FAppear& Appear,
				FAppearAnim& AppearAnim)
			{
				if (AppearAnim.animTime == 0)
				{
					// 状态机
					Animation.PreviousSubjectState = ESubjectState::Dirty;
					Animation.SubjectState = ESubjectState::Appearing;
				}
				
				if (AppearAnim.animTime >= Appear.Duration)
				{
					Subject.RemoveTraitDeferred<FAppearAnim>();
				}

				AppearAnim.animTime += SafeDeltaTime;

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 出生淡入 | Dissolve In
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentAppearDissolve");

		auto Chain = Mechanism->EnchainSolid(AgentAppearDissolveFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

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

				AppearDissolve.dissolveTime += SafeDeltaTime;

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion


	//----------------------攻击 | Attack-------------------------

	// 索敌 | Trace
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentTrace");

		// Trace Player 0
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

		// Trace By Filter
		auto Chain = Mechanism->EnchainSolid(AgentTraceFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, 200, ThreadsCount, BatchSize);

		TArray<FValidSubjects> ValidSubjectsArray;
		ValidSubjectsArray.SetNum(ThreadsCount);

		// A workaround. Apparatus does not expose the array of iterables, so we have to gather manually
		Chain->OperateConcurrently([&](FSolidSubjectHandle Subject, FTrace& Trace)
		{
			if (!Trace.bEnable)
			{
				Trace.TraceResult = FSubjectHandle();
				return;
			}

			bool bShouldTrace = false;

			if (Trace.TimeLeft <= 0)
			{
				Trace.TimeLeft = Trace.CoolDown;
				bShouldTrace = true;
			}
			else
			{
				Trace.TimeLeft -= SafeDeltaTime;
			}

			if (bShouldTrace)// we add iterables into separate arrays and then apend them.
			{
				uint32 ThreadId = FPlatformTLS::GetCurrentThreadId();
				uint32 index = ThreadId % ThreadsCount;// this may not evenly distribute, but well enough

				if (LIKELY(ValidSubjectsArray.IsValidIndex(index)))
				{
					ValidSubjectsArray[index].Lock();// we lock child arrays individually
					ValidSubjectsArray[index].Subjects.Add(Subject);
					ValidSubjectsArray[index].Unlock();
				}
			}

		}, ThreadsCount, BatchSize);

		TArray<FSolidSubjectHandle> ValidSubjects;

		for (auto& CurrentArray : ValidSubjectsArray)
		{
			ValidSubjects.Append(CurrentArray.Subjects);
		}

		// 并行处理筛选结果
		ParallelFor(ValidSubjects.Num(), [&](int32 Index)
		{
			FSolidSubjectHandle Subject = ValidSubjects[Index];

			FLocated& Located = Subject.GetTraitRef<FLocated>();
			FDirected& Directed = Subject.GetTraitRef<FDirected>();
			FCollider& Collider = Subject.GetTraitRef<FCollider>();

			FTrace& Trace = Subject.GetTraitRef<FTrace>();
			FSleep& Sleep = Subject.GetTraitRef<FSleep>();
			FPatrol& Patrol = Subject.GetTraitRef<FPatrol>();

			const bool bIsSleeping = Subject.HasTrait<FSleeping>();
			const bool bIsPatrolling = Subject.HasTrait<FPatrolling>();

			float FinalRange = 0;
			float FinalAngle = 0;
			bool FinalCheckVisibility = false;

			if (bIsSleeping)
			{
				FinalRange = Sleep.TraceRadius;
				FinalAngle = Sleep.TraceAngle;
				FinalCheckVisibility = Sleep.bCheckVisibility;
			}
			else if (bIsPatrolling)
			{
				FinalRange = Patrol.TraceRadius;
				FinalAngle = Patrol.TraceAngle;
				FinalCheckVisibility = Patrol.bCheckVisibility;
			}
			else
			{
				FinalRange = Trace.TraceRadius;
				FinalAngle = Trace.TraceAngle;
				FinalCheckVisibility = Trace.bCheckVisibility;
			}

			switch (Trace.Mode)
			{
				case ETraceMode::TargetIsPlayer_0:
				{
					Trace.TraceResult = FSubjectHandle();

					if (bPlayerIsValid)
					{
						// 计算目标半径和实际距离
						float PlayerRadius = PlayerHandle.HasTrait<FCollider>() ? PlayerHandle.GetTrait<FCollider>().Radius : 0;
						float Distance = FMath::Clamp(FVector::Dist(Located.Location, PlayerLocation) - PlayerRadius, 0, FLT_MAX);// we only take into account the radius of the trace target

						// 距离检查
						if (Distance <= FinalRange)
						{
							// 角度检查
							const FVector ToPlayerDir = (PlayerLocation - Located.Location).GetSafeNormal();
							const float DotValue = FVector::DotProduct(Directed.Direction, ToPlayerDir);
							const float AngleDiff = FMath::RadiansToDegrees(FMath::Acos(DotValue));

							if (AngleDiff <= FinalAngle * 0.5f)
							{
								if (FinalCheckVisibility && IsValid(Trace.NeighborGrid))
								{
									bool Hit = false;
									FTraceResult Result;
									Trace.NeighborGrid->SphereSweepForObstacle(Located.Location, PlayerLocation, Collider.Radius, Hit, Result);

									if (!Hit)
									{
										Trace.TraceResult = PlayerHandle;
									}
								}
								else
								{
									Trace.TraceResult = PlayerHandle;
								}
							}
						}
					}
					break;
				}

				case ETraceMode::SectorTraceByTraits:
				{
					Trace.TraceResult = FSubjectHandle();

					if (LIKELY(IsValid(Trace.NeighborGrid)))
					{
						FFilter TargetFilter;
						bool Hit;
						TArray<FTraceResult> Results;

						TargetFilter.Include(Trace.IncludeTraits);
						TargetFilter.Exclude(Trace.ExcludeTraits);

						// 使用扇形检测替换球体检测
						const FVector TraceDirection = Directed.Direction.GetSafeNormal2D();
						const float TraceHeight = Collider.Radius * 2.0f; // 根据实际需求调整高度

						// ignore self
						FSubjectArray IgnoreList;
						IgnoreList.Subjects.Add(FSubjectHandle(Subject));

						Trace.NeighborGrid->SectorTraceForSubjects
						(
							1,
							Located.Location,   // 检测原点
							FinalRange,         // 检测半径
							TraceHeight,        // 检测高度
							TraceDirection,     // 扇形方向
							FinalAngle,         // 扇形角度
							FinalCheckVisibility,
							Located.Location,
							Collider.Radius,
							ESortMode::NearToFar,
							Located.Location,
							IgnoreList,
							TargetFilter,       // 过滤条件
							Hit,
							Results              // 输出结果
						);

						// 直接使用结果（扇形检测已包含角度验证）
						if (Hit && Results[0].Subject.IsValid())
						{
							Trace.TraceResult = Results[0].Subject;
						}
					}
					break;
				}
			}

			// if we have a valid target, we stop sleeping or patrolling
			if (Trace.TraceResult.IsValid())
			{
				if (bIsSleeping)
				{
					Subject.RemoveTraitDeferred<FSleeping>();
				}

				if (bIsPatrolling)
				{
					Subject.RemoveTraitDeferred<FPatrolling>();
				}
			}
		});

		Mechanism->ApplyDeferreds();
	}
	#pragma endregion

	// 攻击触发 | Trigger Attack
	#pragma region 
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentAttackMain");

		auto Chain = Mechanism->EnchainSolid(AgentAttackFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				const FLocated& Located,
				FDirected Directed,
				FAttack& Attack,
				FTrace& Trace,
				FCollider& Collider)
			{
				TRACE_CPUPROFILER_EVENT_SCOPE_STR("Do AgentAttackMain");
				if (!Attack.bEnable) return;

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

						// 触发攻击
						if (DistToTarget <= Attack.Range && DeltaYaw <= Attack.AngleTolerance && TargetHealth > 0)
						{
							Subject.SetTraitDeferred(FAttacking{ 0.f, EAttackState::PreCast });
						}
					}
				}

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 攻击过程 | Do Attack
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentAttacking");

		auto Chain = Mechanism->EnchainSolid(AgentAttackingFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FLocated& Located,
				FRendering& Rendering,
				FAnimation& Animation,
				FAttack& Attack,
				FAttacking& Attacking,
				FMoving& Moving,
				FDirected& Directed,
				FTrace& Trace,
				FDebuff& Debuff,
				FDamage& Damage,
				FDefence& Defence)
			{
				// First Execute
				if (UNLIKELY(Attacking.Time == 0))
				{
					// Actor
					for (FActorSpawnConfig Config : Attack.SpawnActor)
					{
						Config.SubjectTrans = FTransform(Directed.Direction.Rotation(), Located.Location, Config.Transform.GetScale3D());
						QueueActor(Config);
					}

					// Fx
					for (FFxConfig Config : Attack.SpawnFx)
					{
						Config.SubjectTrans = FTransform(Directed.Direction.Rotation(), Located.Location, Config.Transform.GetScale3D());
						QueueFx(Config);
					}

					// Sound
					for (FSoundConfig Config : Attack.PlaySound)
					{
						Config.SubjectTrans = FTransform(Directed.Direction.Rotation(), Located.Location, Config.Transform.GetScale3D());
						QueueSound(Config);
					}

					// Animation
					Animation.SubjectState = ESubjectState::Attacking;
					Animation.PreviousSubjectState = ESubjectState::Dirty;
				}

				// 到达造成伤害的时间点并且本轮之前也没有攻击过，造成一次伤害
				if (Attacking.Time >= Attack.TimeOfHit && Attacking.Time < Attack.DurationPerRound)
				{
					if (Attacking.State == EAttackState::PreCast)
					{
						Attacking.State = EAttackState::PostCast;

						if (Trace.TraceResult.IsValid() && Trace.TraceResult.HasTrait<FLocated>())
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

							TArray<FDmgResult> DmgResults;

							// Melee ATK
							if (Attack.TimeOfHitAction == EAttackMode::ApplyDMG)
							{
								if (!Trace.TraceResult.HasTrait<FDying>())
								{
									if (Distance <= Attack.Range && Angle <= Attack.AngleTolerance)
									{
										FDmgSphere DmgSphere = { Damage.Damage,Damage.KineticDmg,Damage.FireDmg,Damage.IceDmg,Damage.PercentDmg,Damage.CritProbability,Damage.CritMult };
										ApplyDamageToSubjects(FSubjectArray{ TArray<FSubjectHandle>{Trace.TraceResult} }, FSubjectArray(), FSubjectHandle{ Subject }, Located.Location, DmgSphere, Debuff, DmgResults);
									}
								}
							}

							// Suicide ATK
							if (Attack.TimeOfHitAction == EAttackMode::SuicideATK)
							{
								if (!Trace.TraceResult.HasTrait<FDying>())
								{
									if (Distance <= Attack.Range)
									{
										FDmgSphere DmgSphere = { Damage.Damage,Damage.KineticDmg,Damage.FireDmg,Damage.IceDmg,Damage.PercentDmg,Damage.CritProbability,Damage.CritMult };
										ApplyDamageToSubjects(FSubjectArray{ TArray<FSubjectHandle>{Trace.TraceResult} }, FSubjectArray(), FSubjectHandle{ Subject }, Located.Location, DmgSphere, Debuff, DmgResults);
									}
								}
								Subject.DespawnDeferred();
							}
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
					Moving.LaunchVelSum = FVector::ZeroVector; // 击退力清零
				}

				if (Defence.bSlowATKSpeedOnFreeze && Subject.HasTrait<FFreezing>())
				{
					Attacking.Time += SafeDeltaTime * (1 - Subject.GetTraitRef<FFreezing, EParadigm::Unsafe>().FreezeStr);
				}
				else
				{
					Attacking.Time += SafeDeltaTime;
				}

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion


	//----------------------受击 | Hit-------------------------

	// 受击发光 | Glow
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentHitGlow");

		auto Chain = Mechanism->EnchainSolid(AgentHitGlowFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

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
					HitGlow.GlowTime += SafeDeltaTime;
				}

				// 计时器完成后删除 Trait
				if (HitGlow.GlowTime >= EndTime)
				{
					Animation.HitGlow = 0; // 重置发光值
					Subject->RemoveTraitDeferred<FHitGlow>(); // 延迟删除 Trait
				}

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 怪物受击形变 | Squeeze Squash
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentJiggle");

		auto Chain = Mechanism->EnchainSolid(AgentJiggleFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FScaled& Scaled,
				FJiggle& Jiggle,
				FHit& Hit,
				FCurves& Curves)
			{
				// 获取曲线
				auto Curve = Curves.HitJiggle.GetRichCurve();

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
				Scaled.renderFactors.X = FMath::Lerp(Scaled.Factors.X, Scaled.Factors.X * Curve->Eval(Jiggle.JiggleTime), Hit.JiggleStr);
				Scaled.renderFactors.Y = FMath::Lerp(Scaled.Factors.Y, Scaled.Factors.Y * Curve->Eval(Jiggle.JiggleTime), Hit.JiggleStr);
				Scaled.renderFactors.Z = FMath::Lerp(Scaled.Factors.Z, Scaled.Factors.Z * (2.f - Curve->Eval(Jiggle.JiggleTime)), Hit.JiggleStr);

				// 更新形变时间
				if (Jiggle.JiggleTime < EndTime)
				{
					Jiggle.JiggleTime += SafeDeltaTime;
				}

				// 计时器完成后删除 Trait
				if (Jiggle.JiggleTime >= EndTime)
				{
					Scaled.renderFactors = Scaled.Factors; // 恢复原始比例
					Subject->RemoveTraitDeferred<FJiggle>(); // 延迟删除 Trait
				}

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 灼烧持续掉血 | Burn Temporal Damage
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentBurning");

		auto Chain = Mechanism->EnchainSolid(AgentBurningFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject, FTemporalDamaging& Temporal)
			{
				// 持续伤害结束或玩家退出，终止执行
				if (Temporal.RemainingTemporalDamage <= 0)
				{
					// 恢复颜色
					if (Temporal.TemporalDamageTarget.IsValid())
					{
						if (Temporal.TemporalDamageTarget.HasTrait<FAnimation>())
						{
							auto& TargetAnimation = Temporal.TemporalDamageTarget.GetTraitRef<FAnimation, EParadigm::Unsafe>();
							TargetAnimation.BurnFx = 0;
						}

						if (Temporal.TemporalDamageTarget.HasTrait<FBurning>())
						{
							auto& TargetBurning = Temporal.TemporalDamageTarget.GetTraitRef<FBurning, EParadigm::Unsafe>();

							if (TargetBurning.RemainingTemporalDamager > 1)
							{
								TargetBurning.Lock();
								TargetBurning.RemainingTemporalDamager--;
								TargetBurning.Unlock();
							}
							else
							{
								Temporal.TemporalDamageTarget.RemoveTraitDeferred<FBurning>();
							}
						}
					}

					Subject.DespawnDeferred();
					return;
				}

				Temporal.TemporalDamageTimeout -= SafeDeltaTime;

				// 倒计时，每0.5秒重置，即每秒造成2段灼烧伤害
				if (Temporal.TemporalDamageTimeout <= 0)
				{
					if (Temporal.TemporalDamageTarget.IsValid())
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

										QueueText(FTextPopConfig( Temporal.TemporalDamageTarget, ClampedDamage, Style, TextPopUp.TextScale, Radius * 1.1, Location ));
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

						Temporal.RemainingTemporalDamage -= Temporal.TotalTemporalDamage * 0.25f;

						if (Temporal.RemainingTemporalDamage > 0.f)
						{
							Temporal.TemporalDamageTimeout = 0.5f;
						}
					}
				}
			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 冰冻减速 | Freeze Slowing
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentFrozen");

		auto Chain = Mechanism->EnchainSolid(AgentFrozenFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FAnimation& Animation,
				FFreezing& Freezing)
			{
				// 更新计时器
				if (Freezing.FreezeTimeout > 0.f)
				{
					Freezing.FreezeTimeout -= SafeDeltaTime;
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
	}
	#pragma endregion

	// 结算伤害 | Settle Damage
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("DecideHealth");

		auto Chain = Mechanism->EnchainSolid(DecideHealthFilter);// it processes hero and prop type too
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FHealth& Health,
				FLocated& Located)
			{
				// 结算伤害
				while (!Health.DamageToTake.IsEmpty() && !Health.DamageInstigator.IsEmpty())
				{
					// 如果怪物死了，跳出循环
					if (Health.Current <= 0) { break; }

					FSubjectHandle Instigator = FSubjectHandle();
					float damageToTake = 0.f;

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
							Subject.GetTraitRef<FMove, EParadigm::Unsafe>().bCanFly = false; // 如果在飞行会掉下来
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

				// 更新血条
				const bool bHasHealthBar = Subject.HasTrait<FHealthBar>();

				if (bHasHealthBar)
				{
					auto& HealthBar = Subject.GetTraitRef<FHealthBar>();

					if (HealthBar.bShowHealthBar)
					{
						HealthBar.TargetRatio = FMath::Clamp(Health.Current / Health.Maximum, 0, 1);
						HealthBar.CurrentRatio = FMath::FInterpTo(HealthBar.CurrentRatio, HealthBar.TargetRatio, SafeDeltaTime, HealthBar.InterpSpeed);// WIP use niagara instead

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
				}

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion


	//----------------------死亡 | Death-------------------------

	// 死亡总 | Death Main
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentDeathMain");

		auto Chain = Mechanism->EnchainSolid(AgentDeathFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FDeath Death,
				FLocated Located,
				FDying& Dying,
				FDirected Directed,
				FTrace& Trace,
				FMove& Move,
				FMoving& Moving)
			{
				if (!Death.bEnable) return;

				if (Dying.Time == 0)
				{
					Dying.Duration = Death.DespawnDelay;

					// Stop attacking
					if (Subject.HasTrait<FAttacking>())
					{
						Subject.RemoveTraitDeferred<FAttacking>();
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

					// Actor
					for (FActorSpawnConfig Config : Death.SpawnActor)
					{
						Config.SubjectTrans = FTransform(Directed.Direction.Rotation(), Located.Location, Config.Transform.GetScale3D());
						QueueActor(Config);
					}

					// Fx
					for (FFxConfig Config : Death.SpawnFx)
					{
						Config.SubjectTrans = FTransform(Directed.Direction.Rotation(), Located.Location, Config.Transform.GetScale3D());
						QueueFx(Config);
					}

					// Sound
					for (FSoundConfig Config : Death.PlaySound)
					{
						Config.SubjectTrans = FTransform(Directed.Direction.Rotation(), Located.Location, Config.Transform.GetScale3D());
						QueueSound(Config);
					}
				}

				if (Dying.Time > Dying.Duration)
				{
					Subject.DespawnDeferred();
				}
				else if (Moving.CurrentVelocity.Size2D() < 10.f && Death.bDisableCollision && !Subject.HasTrait<FCorpse>())
				{
					Subject.SetTraitDeferred(FCorpse{});
				}

				Dying.Time += SafeDeltaTime;

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 死亡消融 | Death Dissolve
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentDeathDissolve");

		auto Chain = Mechanism->EnchainSolid(AgentDeathDissolveFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

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
				DeathDissolve.dissolveTime += SafeDeltaTime;

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 死亡动画 | Death Anim
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentDeathAnim");

		auto Chain = Mechanism->EnchainSolid(AgentDeathAnimFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

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

				DeathAnim.animTime += SafeDeltaTime;

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion


	//----------------------移动 | Move-----------------------

	// 速度覆盖 | Override Max Speed
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("SpeedLimitOverride");

		auto Chain = Mechanism->EnchainSolid(SpeedLimitOverrideFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FCollider Collider,
				FLocated Located,
				FSphereObstacle& SphereObstacle)
			{
				if (UNLIKELY(!IsValid(SphereObstacle.NeighborGrid))) return;

				if (!SphereObstacle.bOverrideSpeedLimit) return;

				bool Hit;
				TArray<FTraceResult> Results;

				SphereObstacle.NeighborGrid->SphereTraceForSubjects
				(
					-1,
					Located.Location,
					Collider.Radius,
					false,
					FVector::ZeroVector,
					0,
					ESortMode::None,
					FVector::ZeroVector,
					FSubjectArray(SphereObstacle.OverridingAgents.Array()),
					FFilter::Make<FAgent, FLocated, FCollider, FMoving, FActivated>(),
					Hit,
					Results
				);

				for (const FTraceResult& Result : Results)
				{
					SphereObstacle.OverridingAgents.Add(Result.Subject);
				}

				TSet<FSubjectHandle> Agents = SphereObstacle.OverridingAgents;

				for (const auto& Agent : Agents)
				{
					if (!Agent.IsValid()) continue;

					float AgentRadius = Agent.GetTrait<FCollider>().Radius;
					float SphereObstacleRadius = Collider.Radius;
					float CombinedRadius = AgentRadius + SphereObstacleRadius;
					FVector AgentLocation = Agent.GetTrait<FLocated>().Location;
					float Distance = FVector::Distance(Located.Location, AgentLocation);

					FMoving& AgentMoving = Agent.GetTraitRef<FMoving, EParadigm::Unsafe>();

					if (Distance < CombinedRadius)
					{
						AgentMoving.Lock();
						AgentMoving.bPushedBack = true;
						AgentMoving.PushBackSpeedOverride = SphereObstacle.NewSpeedLimit;
						AgentMoving.Unlock();
					}
					else if (Distance >= CombinedRadius/* * 1.25f*/)
					{
						AgentMoving.Lock();
						AgentMoving.bPushedBack = false;
						AgentMoving.Unlock();
						SphereObstacle.OverridingAgents.Remove(Agent);
					}
				}

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 巡逻 | Patrol
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentPatrol");

		auto Chain = Mechanism->EnchainSolid(AgentPatrolFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		// Lambda for handling patrol reset operations
		auto ResetPatrol = [](FPatrol& Patrol, FPatrolling& Patrolling, const FLocated& Located)
			{
				// Reset timer values
				Patrolling.MoveTimeLeft = Patrol.MaxMovingTime;
				Patrolling.WaitTimeLeft = Patrol.CoolDown;

				// Reset origin based on mode
				if (Patrol.OriginMode == EPatrolOriginMode::Previous)
				{
					Patrol.Origin = Located.Location; // use current location
				}
				else
				{
					Patrol.Origin = Located.InitialLocation; // use initial location
				}
			};

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FLocated& Located,
				FTrace& Trace,
				FPatrol& Patrol,
				FCollider& Collider,
				FMove& Move,
				FMoving& Moving)
			{
				if (!Patrol.bEnable) return;

				// 巡逻逻辑
				bool HasPatrolling = Subject.HasTrait<FPatrolling>();

				if (HasPatrolling)
				{
					auto& Patrolling = Subject.GetTraitRef<FPatrolling, EParadigm::Unsafe>();

					// 检查是否到达目标点
					const bool bHasArrived = FVector::Dist2D(Located.Location, Moving.Goal) < Patrol.AcceptanceRadius;

					if (bHasArrived)
					{
						// 到达目标点后的等待逻辑
						if (Patrolling.WaitTimeLeft <= 0.f)
						{
							ResetPatrol(Patrol, Patrolling, Located);
							Moving.Goal = FindNewPatrolGoalLocation(Patrol, Collider, Trace, Located, 3);
						}
						else
						{
							Patrolling.WaitTimeLeft -= SafeDeltaTime;
						}
					}
					else
					{
						// 移动超时逻辑
						if (Patrolling.MoveTimeLeft <= 0.f)
						{
							ResetPatrol(Patrol, Patrolling, Located);
							Moving.Goal = FindNewPatrolGoalLocation(Patrol, Collider, Trace, Located, 3);
						}
						else
						{
							Patrolling.MoveTimeLeft -= SafeDeltaTime;
						}
					}
				}

				// 重新进入巡逻状态的逻辑
				else if (!Trace.TraceResult.IsValid() && Patrol.OnLostTarget == EPatrolRecoverMode::Patrol)
				{
					FPatrolling NewPatrolling;
					ResetPatrol(Patrol, NewPatrolling, Located);
					Subject.SetTraitDeferred(NewPatrolling);
				}

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 加载流场 | Load Flow Field
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("LoadFlowfield");

		FFilter Filter = FFilter::Make<FActivated>();
		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, 1, ThreadsCount, BatchSize);

		// filter out those need reload and mark them
		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject)
			{
				Subject.SetFlag(ReloadFlowFieldFlag, false);

				const bool bHasNavigation = Subject.HasTrait<FNavigation>();
				const bool bHasBindFlowField = Subject.HasTrait<FBindFlowField>();

				if (bHasNavigation)
				{
					auto& Navigation = Subject.GetTraitRef<FNavigation>();

					if (UNLIKELY(Navigation.FlowFieldToUse != Navigation.PreviousFlowFieldToUse))
					{
						Navigation.PreviousFlowFieldToUse = Navigation.FlowFieldToUse;

						if (Navigation.FlowFieldToUse.IsValid())
						{
							Subject.SetFlag(ReloadFlowFieldFlag, true);
						}
					}
				}

				if (bHasBindFlowField)
				{
					auto& BindFlowField = Subject.GetTraitRef<FBindFlowField>();

					if (UNLIKELY(BindFlowField.FlowFieldToBind != BindFlowField.PreviousFlowFieldToBind))
					{
						BindFlowField.PreviousFlowFieldToBind = BindFlowField.FlowFieldToBind;

						if (BindFlowField.FlowFieldToBind.IsValid())
						{
							Subject.SetFlag(ReloadFlowFieldFlag, true);
						}
					}
				}

			}, ThreadsCount, BatchSize);

		// do reload, this only works on game thread
		Mechanism->Operate<FUnsafeChain>(Filter.IncludeFlag(ReloadFlowFieldFlag),
			[&](FUnsafeSubjectHandle Subject)
			{
				const bool bHasNavigation = Subject.HasTrait<FNavigation>();
				const bool bHasBindFlowField = Subject.HasTrait<FBindFlowField>();

				if (bHasNavigation)
				{
					auto& Navigation = Subject.GetTraitRef<FNavigation>();
					Navigation.FlowField = Navigation.FlowFieldToUse.LoadSynchronous();
				}

				if (bHasBindFlowField)
				{
					auto& BindFlowField = Subject.GetTraitRef<FBindFlowField>();
					BindFlowField.FlowField = BindFlowField.FlowFieldToBind.LoadSynchronous();
				}
			});
	}
	#pragma endregion

	// 移动 | Move
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentMove");

		auto Chain = Mechanism->EnchainSolid(AgentMoveFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FMove& Move,
				FMoving& Moving,
				FAvoidance& Avoidance,
				FAvoiding& Avoiding,
				FDirected& Directed,
				FLocated& Located,
				FTrace& Trace,
				FNavigation& Navigation,
				FCollider& Collider,
				FDefence& Defence,
				FPatrol& Patrol,
				FGridData& GridData)
			{
				// 死亡区域检测
				if (Located.Location.Z < Move.KillZ)
				{
					Subject.DespawnDeferred();
					return;
				}

				const bool bIsValidFF = IsValid(Navigation.FlowField);

				// 必须要有一个流场
				if (!bIsValidFF) return;

				const bool bIsAppearing = Subject.HasTrait<FAppearing>();
				const bool bIsSleeping = Subject.HasTrait<FSleeping>();
				const bool bIsPatrolling = Subject.HasTrait<FPatrolling>();
				const bool bIsAttacking = Subject.HasTrait<FAttacking>();
				const bool bIsFreezing = Subject.HasTrait<FFreezing>();
				const bool bIsDying = Subject.HasTrait<FDying>();

				const bool bIsValidTraceResult = Trace.TraceResult.IsValid();
				const bool bIsTraceResultHasLocated = bIsValidTraceResult ? Trace.TraceResult.HasTrait<FLocated>() : false;
				const bool bIsTraceResultHasBindFlowField = bIsValidTraceResult ? Trace.TraceResult.HasTrait<FBindFlowField>() : false;

				// 初始化
				FVector& AgentLocation = Located.Location;
				FVector DesiredMoveDirection = FVector::ZeroVector;

				// 移动减速比率
				Moving.MoveSpeedMult = 1;

				// 默认流场, 必须获取因为之后要用到地面高度
				bool bInside_BaseFF;
				FCellStruct& Cell_BaseFF = Navigation.FlowField->GetCellAtLocation(AgentLocation, bInside_BaseFF);

				//------------------------------ 击飞 ----------------------------//

				if (Moving.LaunchVelSum != FVector::ZeroVector)// add pending deltaV into current V
				{
					Moving.CurrentVelocity += Moving.LaunchVelSum * (1 - (bIsDying ? Defence.KineticDebuffImmuneDead : Defence.KineticDebuffImmuneAlive));

					FVector XYDir = Moving.CurrentVelocity.GetSafeNormal2D();
					float XYSpeed = Moving.CurrentVelocity.Size2D();
					XYSpeed = FMath::Clamp(XYSpeed, 0, Move.MoveSpeed + Defence.KineticMaxImpulse);
					FVector XYVelocity = XYSpeed * XYDir;

					float ZSpeed = Moving.CurrentVelocity.Z;
					ZSpeed = FMath::Clamp(ZSpeed, -Defence.KineticMaxImpulse, Defence.KineticMaxImpulse);

					Moving.CurrentVelocity = FVector(XYVelocity.X, XYVelocity.Y, ZSpeed);
					Moving.LaunchVelSum = FVector::ZeroVector;
					Moving.bLaunching = true;
				}

				if (Moving.bLaunching)// switch launching state by vV
				{
					if (Moving.CurrentVelocity.Size2D() < 100.f)
					{
						Moving.bLaunching = false;
					}
				}

				//---------------------------- 速度方向 ----------------------------//

				if (Move.bEnable && !bIsAppearing)// Appearing不寻路
				{
					if (bIsPatrolling)// Patrolling直接向目标点移动，之后要做个体寻路
					{
						DesiredMoveDirection = (Moving.Goal - AgentLocation).GetSafeNormal2D();
					}
					else
					{
						if (!bIsValidTraceResult) // 无攻击目标
						{
							if (bInside_BaseFF)
							{
								Moving.Goal = Navigation.FlowField->goalLocation;
								DesiredMoveDirection = Cell_BaseFF.dir.GetSafeNormal2D();
							}
							else
							{
								Moving.MoveSpeedMult = 0;
							}
						}
						else
						{
							// 直接向攻击目标移动，之后要做个体寻路
							auto ApproachTraceResultDirectly = [&]()
							{
								if (bIsTraceResultHasLocated)
								{
									Moving.Goal = Trace.TraceResult.GetTrait<FLocated>().Location;
									DesiredMoveDirection = (Moving.Goal - AgentLocation).GetSafeNormal2D();
								}
								else
								{
									Moving.MoveSpeedMult = 0;
								}
							};

							if (bIsTraceResultHasLocated)
							{
								Moving.Goal = Trace.TraceResult.GetTrait<FLocated>().Location;
							}

							if (bIsTraceResultHasBindFlowField)
							{
								AFlowField* BindFlowField = Trace.TraceResult.GetTrait<FBindFlowField>().FlowField;

								if (IsValid(BindFlowField)) // 从目标获取指向目标的流场
								{
									bool bInside_TargetFF;
									FCellStruct& Cell_TargetFF = BindFlowField->GetCellAtLocation(AgentLocation, bInside_TargetFF);

									if (bInside_TargetFF)
									{
										Moving.Goal = BindFlowField->goalLocation;
										DesiredMoveDirection = Cell_TargetFF.dir.GetSafeNormal2D();
									}
									else
									{
										ApproachTraceResultDirectly();
									}
								}
								else
								{
									ApproachTraceResultDirectly();
								}
							}
							else
							{
								ApproachTraceResultDirectly();
							}
						}
					}
				}		

				//---------------------------- 速度大小 ----------------------------//

				float DistanceToGoal = FVector::Dist2D(AgentLocation, Moving.Goal);

				// stop under these circumstances
				if (!Move.bEnable || Moving.bLaunching || Moving.bFalling || bIsAppearing || bIsSleeping  || bIsDying)
				{
					Moving.MoveSpeedMult = 0.0f;
				}
				
				// stop after reaching patrol goal
				if (bIsPatrolling)
				{
					if (DistanceToGoal <= Patrol.AcceptanceRadius)
					{
						Moving.MoveSpeedMult = 0;
					}
					else
					{
						Moving.MoveSpeedMult *= Patrol.MoveSpeedMult;
					}
				}
				
				// stop during attack
				if (bIsAttacking)
				{
					EAttackState State = Subject.GetTraitRef<FAttacking, EParadigm::Unsafe>().State;

					if (State != EAttackState::Cooling)
					{
						Moving.MoveSpeedMult = 0.0f;
					}
				}
				
				// slow down when frozen
				if (bIsFreezing)
				{
					const auto& Freezing = Subject.GetTraitRef<FFreezing, EParadigm::Unsafe>();
					Moving.MoveSpeedMult *= 1.0f - Freezing.FreezeStr;
				}

				// 朝向-移动方向夹角 插值
				float DotProduct = FVector::DotProduct(Directed.Direction, Moving.CurrentVelocity.GetSafeNormal2D());
				float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

				const TRange<float> TurnInputRange(Move.MoveSpeedRangeMapByAngle.X, Move.MoveSpeedRangeMapByAngle.Z);
				const TRange<float> TurnOutputRange(Move.MoveSpeedRangeMapByAngle.Y, Move.MoveSpeedRangeMapByAngle.W);

				Moving.MoveSpeedMult *= FMath::GetMappedRangeValueClamped(TurnInputRange, TurnOutputRange, AngleDegrees);

				// 速度-与目标距离 插值
				if (!bIsPatrolling)
				{
					float OtherRadius = Trace.TraceResult.HasTrait<FCollider>() ? Trace.TraceResult.GetTrait<FCollider>().Radius : 0;
					const float DistanceToTarget = FMath::Clamp(DistanceToGoal - OtherRadius, 0, FLT_MAX);

					if (DistanceToTarget <= Move.AcceptanceRadius)
					{
						Moving.MoveSpeedMult = 0;
					}
					else
					{
						const TRange<float> MoveInputRange(Move.MoveSpeedRangeMapByDist.X, Move.MoveSpeedRangeMapByDist.Z);
						const TRange<float> MoveOutputRange(Move.MoveSpeedRangeMapByDist.Y, Move.MoveSpeedRangeMapByDist.W);

						Moving.MoveSpeedMult *= FMath::GetMappedRangeValueClamped(MoveInputRange, MoveOutputRange, DistanceToTarget);
					}
				}

				//---------------------------- 水平速度 ----------------------------//

				float DesiredSpeed = Move.MoveSpeed * Moving.MoveSpeedMult;
				FVector DesiredVelocity = DesiredSpeed * DesiredMoveDirection;
				FVector CurrentVelocity = Moving.CurrentVelocity * FVector(1, 1, 0);
				FVector InterpedVelocity = FMath::VInterpConstantTo(CurrentVelocity, DesiredVelocity, DeltaTime, Move.MoveAcceleration);
				Moving.DesiredVelocity = DesiredVelocity;

				//---------------------------- 执行朝向 ----------------------------//

				// 计算朝向减速比率
				Moving.TurnSpeedMult = 1;

				// 这些情况不转向
				if (!Move.bEnable || Moving.bFalling || Moving.bLaunching || bIsAppearing || bIsSleeping || bIsAttacking || bIsDying || (bIsPatrolling && DistanceToGoal <= Patrol.AcceptanceRadius))
				{
					Moving.TurnSpeedMult = 0;
				}

				// 冰冻时减慢转向速度
				if (bIsFreezing)
				{
					const auto& Freezing = Subject.GetTraitRef<FFreezing, EParadigm::Unsafe>();
					Moving.TurnSpeedMult *= 1.0f - Freezing.FreezeStr;
				}

				if (Moving.TurnSpeedMult > 0.f)
				{
					// 计算速度比例和混合因子
					float SpeedRatio = FMath::Clamp(Moving.CurrentVelocity.Size2D() / Move.MoveSpeed, 0.0f, 1.0f);
					float BlendFactor = FMath::Pow(SpeedRatio, 2.0f); // 使用平方使低速时更倾向于平均速度

					// 混合当前速度和平均速度
					FVector LerpedVelocity = FMath::Lerp(Moving.AverageVelocity, Moving.CurrentVelocity, BlendFactor);
					FVector VelocityDirection = LerpedVelocity.GetSafeNormal2D();
					float VelocitySize = LerpedVelocity.Size2D();

					// 朝向-移动方向夹角 插值
					DotProduct = FVector::DotProduct(Moving.DesiredVelocity.GetSafeNormal2D(), VelocityDirection);
					AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

					float bInvertSign = AngleDegrees > 90.f && Move.TurnMode == EOrientMode::ToMovementForwardAndBackward ? -1.f : 1.f;

					FVector OrientDirection = Move.TurnMode == EOrientMode::ToPath ? DesiredVelocity.GetSafeNormal2D() : VelocityDirection * bInvertSign;

					// 执行转向插值
					FRotator CurrentRot = Directed.Direction.ToOrientationRotator();
					FRotator TargetRot = OrientDirection.ToOrientationRotator();

					// 计算当前与目标的Yaw差
					float CurrentYaw = CurrentRot.Yaw;
					float TargetYaw = TargetRot.Yaw;
					float DeltaYaw = FRotator::NormalizeAxis(TargetYaw - CurrentYaw);

					// 小角度容差
					const float ANGLE_TOLERANCE = 0.1f;

					if (FMath::Abs(DeltaYaw) < ANGLE_TOLERANCE)
					{
						// 已经对准目标，停止旋转
						Moving.CurrentAngularVelocity = 0.0f;
						CurrentRot.Yaw = TargetYaw;
					}
					else
					{
						// 旋转方向
						const float Dir = FMath::Sign(DeltaYaw);
						float Acceleration = 0.0f;

						// 速度方向判断
						if (FMath::Sign(Moving.CurrentAngularVelocity) == Dir)
						{
							// 方向正确时的减速判断
							const float CurrentSpeed = FMath::Abs(Moving.CurrentAngularVelocity);
							const float StopDistance = (CurrentSpeed * CurrentSpeed) / (2 * Move.TurnAcceleration);

							if (StopDistance >= FMath::Abs(DeltaYaw))
							{
								// 需要减速停止
								Acceleration = -Dir * Move.TurnAcceleration;
							}
							else if (CurrentSpeed < Move.TurnSpeed)
							{
								// 可以继续加速
								Acceleration = Dir * Move.TurnAcceleration;
							}
						}
						else
						{
							// 方向错误时先减速到0
							if (!FMath::IsNearlyZero(Moving.CurrentAngularVelocity, 0.1f))
							{
								Acceleration = -FMath::Sign(Moving.CurrentAngularVelocity) * Move.TurnAcceleration;
							}
							else
							{
								// 静止状态直接开始加速
								Acceleration = Dir * Move.TurnAcceleration;
							}
						}

						// 计算新角速度
						float NewAngularVelocity = Moving.CurrentAngularVelocity + Acceleration * DeltaTime;
						NewAngularVelocity = FMath::Clamp(NewAngularVelocity, -Move.TurnSpeed, Move.TurnSpeed);

						// 使用平均速度计算实际转动角度
						const float AvgAngularVelocity = 0.5f * (Moving.CurrentAngularVelocity + NewAngularVelocity);
						float AppliedDeltaYaw = AvgAngularVelocity * DeltaTime;

						// 防止角度过冲
						if (FMath::Abs(AppliedDeltaYaw) > FMath::Abs(DeltaYaw))
						{
							AppliedDeltaYaw = DeltaYaw;
							NewAngularVelocity = 0.0f; // 到达目标后停止
						}

						// 应用旋转
						CurrentRot.Yaw = FRotator::NormalizeAxis(CurrentRot.Yaw + AppliedDeltaYaw);
						Moving.CurrentAngularVelocity = NewAngularVelocity * Moving.TurnSpeedMult;
					}

					// 保持Pitch和Roll不变
					CurrentRot.Pitch = TargetRot.Pitch;
					CurrentRot.Roll = TargetRot.Roll;

					if (VelocitySize > Move.MoveSpeed * 0.05f)
					{
						Directed.Direction = CurrentRot.Vector();
					}
				}

				//----------------------------- 避障 ----------------------------//

				const auto NeighborGrid = Trace.NeighborGrid;

				if (LIKELY(Avoidance.bEnable) && LIKELY(IsValid(NeighborGrid)))
				{
					const auto& SelfLocation = Located.Location;
					const auto SelfRadius = Collider.Radius;
					const auto TraceDist = Avoidance.TraceDist;
					const float TotalRangeSqr = FMath::Square(SelfRadius + TraceDist);
					const int32 MaxNeighbors = Avoidance.MaxNeighbors;

					// Avoid Subject Neighbors
					{
						FFilter SubjectFilter = SubjectFilterBase;

						// 碰撞组
						if (!Avoidance.IgnoreGroups.IsEmpty())
						{
							const int32 ClampedGroups = FMath::Clamp(Avoidance.IgnoreGroups.Num(), 0, 9);

							for (int32 i = 0; i < ClampedGroups; ++i)
							{
								UBattleFrameFunctionLibraryRT::ExcludeAvoGroupTraitByIndex(Avoidance.IgnoreGroups[i], SubjectFilter);
							}
						}

						if (UNLIKELY(Subject.HasTrait<FDying>()))
						{
							SubjectFilter.Include<FDying>();// dying subject only collide with dying subjects
						}

						const FVector SubjectRange3D(TraceDist + SelfRadius, TraceDist + SelfRadius, SelfRadius);
						TArray<FIntVector> NeighbourCellCoords = NeighborGrid->GetNeighborCells(SelfLocation, SubjectRange3D);

						// 使用最大堆收集最近的SubjectNeighbors
						auto SubjectCompare = [&](const FGridData& A, const FGridData& B)
						{
							return FVector::DistSquared(SelfLocation, FVector(A.Location)) > FVector::DistSquared(SelfLocation, FVector(B.Location));
						};

						TArray<FGridData> SubjectNeighbors;
						SubjectNeighbors.Reserve(MaxNeighbors + 1);

						TSet<uint32> SeenHashes;
						SeenHashes.Reserve(16);

						SeenHashes.Add(GridData.SubjectHash);

						// this for loop is the most expensive code of all
						for (const auto& Coord : NeighbourCellCoords)
						{
							const auto& Subjects = NeighborGrid->At(Coord).Subjects;

							for (const auto& Data : Subjects)
							{
								// these check are arranged so for cache optimization
								// 距离检查
								const float DistSqr = FVector::DistSquared(SelfLocation, FVector(Data.Location));
								const float RadiusSqr = FMath::Square(Data.Radius) + TotalRangeSqr;

								if (DistSqr > RadiusSqr) continue;

								// 去重
								if (UNLIKELY(SeenHashes.Contains(Data.SubjectHash))) continue;

								// Filter By Traits
								if (UNLIKELY(!Data.SubjectHandle.Matches(SubjectFilter))) continue;

								SeenHashes.Add(Data.SubjectHash);

								// we limit the amount of subjects. we keep the nearest MaxNeighbors amount of neighbors
								// 动态维护堆
								if (LIKELY(SubjectNeighbors.Num() < MaxNeighbors))
								{
									SubjectNeighbors.HeapPush(Data, SubjectCompare);
								}
								else
								{
									const auto& HeapTop = SubjectNeighbors.HeapTop();
									const float HeapTopDist = FVector::DistSquared(SelfLocation, FVector(HeapTop.Location));

									if (UNLIKELY(DistSqr < HeapTopDist))
									{
										// 弹出时同步移除哈希记录
										SeenHashes.Remove(HeapTop.SubjectHash);
										SubjectNeighbors.HeapPopDiscard(SubjectCompare);
										SubjectNeighbors.HeapPush(Data, SubjectCompare);
									}
								}
							}
						}

						Avoidance.MaxSpeed = Moving.DesiredVelocity.Size2D();
						Avoidance.DesiredVelocity = RVO::Vector2(Moving.DesiredVelocity.X, Moving.DesiredVelocity.Y);
						Avoiding.CurrentVelocity = RVO::Vector2(Moving.CurrentVelocity.X, Moving.CurrentVelocity.Y);

						// suggest the velocity to avoid collision
						TArray<FGridData> EmptyArray;
						ComputeAvoidingVelocity(Avoidance, Avoiding, SubjectNeighbors, EmptyArray, DeltaTime);

						// apply velocity
						if (LIKELY(!Moving.bFalling && !Moving.bLaunching))
						{
							FVector AvoidingVelocity(Avoidance.AvoidingVelocity.x(), Avoidance.AvoidingVelocity.y(), 0);
							Moving.CurrentVelocity = FVector(AvoidingVelocity.X, AvoidingVelocity.Y, Moving.CurrentVelocity.Z);
						}
						else if (Moving.bLaunching)
						{
							FVector DeceleratingVelocity = FMath::VInterpConstantTo(Moving.CurrentVelocity * FVector(1, 1, 0), FVector::ZeroVector, DeltaTime, Move.MoveDeceleration);
							Moving.CurrentVelocity = FVector(DeceleratingVelocity.X, DeceleratingVelocity.Y, Moving.CurrentVelocity.Z);
						}
					}

					// Avoid Obstacle Neighbors
					{
						Avoidance.MaxSpeed = Moving.bPushedBack ? FMath::Max(Moving.CurrentVelocity.Size2D(), Moving.PushBackSpeedOverride) : Moving.CurrentVelocity.Size2D();
						Avoidance.DesiredVelocity = RVO::Vector2(Moving.CurrentVelocity.X, Moving.CurrentVelocity.Y);
						Avoiding.CurrentVelocity = RVO::Vector2(Moving.CurrentVelocity.X, Moving.CurrentVelocity.Y);

						const float ObstacleRange = Avoidance.RVO_TimeHorizon_Obstacle * Avoidance.MaxSpeed + Avoiding.Radius;
						const FVector ObstacleRange3D(ObstacleRange, ObstacleRange, Avoiding.Radius);
						TArray<FIntVector> ObstacleCellCoords = NeighborGrid->GetNeighborCells(SelfLocation, ObstacleRange3D);

						TSet<FGridData> ValidSphereObstacleNeighbors;
						TSet<FGridData> ValidBoxObstacleNeighbors;

						ValidSphereObstacleNeighbors.Reserve(MaxNeighbors);
						ValidBoxObstacleNeighbors.Reserve(MaxNeighbors);

						for (const FIntVector& Coord : ObstacleCellCoords)
						{
							const auto& Cell = NeighborGrid->At(Coord);

							// 定义处理 SphereObstacles 的 Lambda 函数
							auto ProcessSphereObstacles = [&](const TArray<FGridData, TInlineAllocator<8>>& Obstacles)
							{
								ValidSphereObstacleNeighbors.Append(Obstacles);
							};

							ProcessSphereObstacles(Cell.SphereObstacles);
							ProcessSphereObstacles(Cell.SphereObstaclesStatic);

							// 定义处理 BoxObstacles 的 Lambda 函数
							auto ProcessBoxObstacles = [&](const TArray<FGridData, TInlineAllocator<8>>& Obstacles)
							{
								for (const FGridData& AvoData : Obstacles)
								{
									const FSubjectHandle ObstacleHandle = AvoData.SubjectHandle;
									if (!ObstacleHandle.IsValid()) continue;
									const auto ObstacleData = ObstacleHandle.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();
									if (!ObstacleData) continue;
									const FSubjectHandle NextObstacleHandle = ObstacleData->nextObstacle_;
									if (!NextObstacleHandle.IsValid()) continue;
									const auto NextObstacleData = NextObstacleHandle.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();
									if (!NextObstacleData) continue;

									const FVector ObstaclePoint = ObstacleData->point3d_;
									const float ObstacleHalfHeight = ObstacleData->height_ * 0.5f;
									const FVector NextPoint = NextObstacleData->point3d_;

									// Z 轴范围检查
									const float ObstacleZMin = ObstaclePoint.Z - ObstacleHalfHeight;
									const float ObstacleZMax = ObstaclePoint.Z + ObstacleHalfHeight;
									const float SubjectZMin = SelfLocation.Z - SelfRadius;
									const float SubjectZMax = SelfLocation.Z + SelfRadius;

									if (SubjectZMax < ObstacleZMin || SubjectZMin > ObstacleZMax) continue;

									// 2D 碰撞检测（RVO）
									RVO::Vector2 currentPos(Located.Location.X, Located.Location.Y);
									RVO::Vector2 obstacleStart(ObstaclePoint.X, ObstaclePoint.Y);
									RVO::Vector2 obstacleEnd(NextPoint.X, NextPoint.Y);

									float leftOfValue = RVO::leftOf(obstacleStart, obstacleEnd, currentPos);

									if (leftOfValue < 0.0f)
									{
										ValidBoxObstacleNeighbors.Add(AvoData);
									}
								}
							};

							ProcessBoxObstacles(Cell.BoxObstacles);
							ProcessBoxObstacles(Cell.BoxObstaclesStatic);
						}

						TArray<FGridData> SphereObstacleNeighbors = ValidSphereObstacleNeighbors.Array();
						TArray<FGridData> BoxObstacleNeighbors = ValidBoxObstacleNeighbors.Array();

						ComputeAvoidingVelocity(Avoidance, Avoiding, SphereObstacleNeighbors, BoxObstacleNeighbors, DeltaTime);

						Moving.CurrentVelocity = FVector(Avoidance.AvoidingVelocity.x(), Avoidance.AvoidingVelocity.y(), Moving.CurrentVelocity.Z);
					}
				}

				// 更新速度历史记录
				if (UNLIKELY(Moving.bShouldInit))
				{
					Moving.Initialize();
				}

				if (UNLIKELY(Moving.TimeLeft <= 0.f))
				{
					Moving.UpdateVelocityHistory(Moving.CurrentVelocity);
				}

				Moving.TimeLeft -= DeltaTime;

				//--------------------------- 垂直速度 -----------------------------//

				if (bIsValidFF)// 没有流场则跳过，因为不知道地面高度，所以不考虑垂直运动
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

						bool bInside;
						FCellStruct& Cell = Navigation.FlowField->GetCellAtLocation(BoundSamplePoint, bInside);

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

						if (Move.bCanFly)
						{
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
								Moving.CurrentVelocity.Z += Move.Gravity * SafeDeltaTime;

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
									FVector BounceDecay = FVector(Move.MoveBounceVelocityDecay.X, Move.MoveBounceVelocityDecay.X, Move.MoveBounceVelocityDecay.Y);
									Moving.CurrentVelocity = Moving.CurrentVelocity * BounceDecay * FVector(1, 1, (FMath::Abs(Moving.CurrentVelocity.Z) > 100.f) ? -1 : 0);// zero out small number
								}

								// 平滑移动到地面
								AgentLocation.Z = FMath::FInterpTo(AgentLocation.Z, CollisionThreshold, SafeDeltaTime, 25.0f);
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
							Moving.CurrentVelocity.Z += Move.Gravity * SafeDeltaTime;

							if (!Moving.bFalling)
							{
								Moving.bFalling = true;
							}
						}
					}
				}
				else
				{
					Moving.CurrentVelocity.Z = 0;
				}

				//--------------------------- 执行位移 -----------------------------//

				Located.PreLocation = Located.Location;
				Located.Location += Moving.CurrentVelocity * DeltaTime;

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	
	//----------------------- 渲染 | Rendering ------------------------

	// 动画状态机 | Anim State Machine
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentStateMachine");

		auto Chain = Mechanism->EnchainSolid(AgentStateMachineFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FAnimation& Anim,
				FAppear& Appear,
				FAttack& Attack,
				FDeath& Death,
				FMove& Move,
				FMoving& Moving)
			{
				// 待机-移动切换 | Idle-Move Switch
				const bool bIsAttacking = Subject.HasTrait<FAttacking>();

				if (bIsAttacking)
				{
					EAttackState State = Subject.GetTraitRef<FAttacking, EParadigm::Unsafe>().State;

					if (State == EAttackState::Cooling)
					{
						const bool IsMoving = Moving.CurrentVelocity.Size2D() > Move.MoveSpeed * 0.05f;
						Anim.SubjectState = IsMoving ? ESubjectState::Moving : ESubjectState::Idle;
					}
				}
				else
				{
					const bool IsMoving = Moving.CurrentVelocity.Size2D() > Move.MoveSpeed * 0.05f;
					Anim.SubjectState = IsMoving ? ESubjectState::Moving : ESubjectState::Idle;
				}

				// 动画状态机 | Anim State Machine
				if (Anim.SubjectState != Anim.PreviousSubjectState && Anim.AnimLerp == 1)
				{
					switch (Anim.SubjectState)
					{
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
							CopyAnimData(Anim);
							Anim.AnimIndex1 = Anim.IndexOfAppearAnim;
							Anim.AnimCurrentTime1 = GetGameTimeSinceCreation();
							Anim.AnimOffsetTime1 = 0;
							Anim.AnimPauseTime1 = Anim.AnimLengthArray[Anim.IndexOfAppearAnim];
							Anim.AnimPlayRate1 = Anim.AnimPauseTime1 / Appear.Duration;
							Anim.AnimLerp = 1;// since appearing is definitely the first anim to play
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
								Anim.AnimPlayRate1 = Anim.IdlePlayRate * (1 - Subject.GetTraitRef<FFreezing, EParadigm::Unsafe>().FreezeStr);
							}
							else
							{
								Anim.AnimPlayRate1 = Anim.IdlePlayRate;
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
								Anim.AnimPlayRate1 = Anim.MovePlayRate * (1 - Subject.GetTraitRef<FFreezing, EParadigm::Unsafe>().FreezeStr);
							}
							else
							{
								Anim.AnimPlayRate1 = Anim.MovePlayRate;
							}
							break;
						}

						case ESubjectState::Attacking:
						{
							CopyAnimData(Anim);
							Anim.AnimIndex1 = Anim.IndexOfAttackAnim;
							Anim.AnimCurrentTime1 = GetGameTimeSinceCreation();
							Anim.AnimOffsetTime1 = 0;
							Anim.AnimPauseTime1 = Anim.AnimLengthArray[Anim.IndexOfAttackAnim];

							if (Subject.HasTrait<FFreezing>())
							{
								Anim.AnimPlayRate1 = Anim.AnimPauseTime1 / Attack.DurationPerRound * (1 - Subject.GetTraitRef<FFreezing, EParadigm::Unsafe>().FreezeStr);
							}
							else
							{
								Anim.AnimPlayRate1 = Anim.AnimPauseTime1 / Attack.DurationPerRound;
							}
							break;
						}

						case ESubjectState::Dying:
						{
							CopyAnimData(Anim);
							Anim.AnimIndex1 = Anim.IndexOfDeathAnim;
							Anim.AnimCurrentTime1 = GetGameTimeSinceCreation();
							Anim.AnimOffsetTime1 = 0;
							Anim.AnimPauseTime1 = Anim.AnimLengthArray[Anim.IndexOfDeathAnim];
							Anim.AnimPlayRate1 = Anim.AnimPauseTime1 / Death.AnimLength;
							break;
						}
					}

					Anim.PreviousSubjectState = Anim.SubjectState;
				}

				Anim.AnimLerp = FMath::Clamp(Anim.AnimLerp + SafeDeltaTime * Anim.LerpSpeed, 0, 1);

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 池初始化 | Init Pooling Info
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("ClearValidTransforms");

		auto Chain = Mechanism->EnchainSolid(RenderBatchFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

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

	// 收集渲染数据 | Gather Render Data
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentRender");

		auto Chain = Mechanism->EnchainSolid(AgentRenderFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FRendering& Rendering,
				FDirected& Directed,
				FScaled& Scaled,
				FLocated& Located,
				FAnimation& Anim,
				FHealth& Health,
				FHealthBar& HealthBar,
				FPoppingText& PoppingText,
				FCollider& Collider)
			{
				FRenderBatchData& Data = Rendering.Renderer.GetTraitRef<FRenderBatchData, EParadigm::Unsafe>();

				FQuat Rotation{ FQuat::Identity };
				Rotation = Directed.Direction.Rotation().Quaternion();

				FVector FinalScale(Data.Scale);
				FinalScale *= Scaled.renderFactors;

				float Radius = Collider.Radius;

				// 在计算转换时减去Radius
				FTransform SubjectTransform(Rotation * Data.OffsetRotation.Quaternion(), Located.Location + Data.OffsetLocation - FVector(0, 0, Radius), FinalScale); // 减去Z轴上的Radius					

				int32 InstanceId = Rendering.InstanceId;

				Data.Lock();

				Data.ValidTransforms[InstanceId] = true;
				Data.Transforms[InstanceId] = SubjectTransform;

				// Transforms
				Data.LocationArray[InstanceId] = SubjectTransform.GetLocation();
				Data.OrientationArray[InstanceId] = SubjectTransform.GetRotation();
				Data.ScaleArray[InstanceId] = SubjectTransform.GetScale3D();

				// Dynamic params 0
				Data.Anim_Index0_Index1_PauseTime0_PauseTime1_Array[InstanceId] = FVector4(Anim.AnimIndex0, Anim.AnimIndex1, Anim.AnimPauseTime0, Anim.AnimPauseTime1);

				// Dynamic params 1
				Data.Anim_TimeStamp0_TimeStamp1_PlayRate0_Playrate1_Array[InstanceId] = FVector4(Anim.AnimCurrentTime0 + Anim.AnimOffsetTime0, Anim.AnimCurrentTime1 + Anim.AnimOffsetTime1, Anim.AnimPlayRate0, Anim.AnimPlayRate1);

				// Pariticle color R
				Data.Anim_Lerp_Array[InstanceId] = Anim.AnimLerp;

				// Dynamic params 2
				Data.Mat_HitGlow_Freeze_Burn_Dissolve_Array[InstanceId] = FVector4(Anim.HitGlow, Anim.FreezeFx, Anim.BurnFx, Anim.Dissolve);

				// HealthBar
				Data.HealthBar_Opacity_CurrentRatio_TargetRatio_Array[InstanceId] = FVector(HealthBar.Opacity, HealthBar.CurrentRatio, HealthBar.TargetRatio);

				// PopText
				Data.Text_Location_Array.Append(PoppingText.TextLocationArray);
				Data.Text_Value_Style_Scale_Offset_Array.Append(PoppingText.Text_Value_Style_Scale_Offset_Array);

				Data.Unlock();

				PoppingText.TextLocationArray.Empty();
				PoppingText.Text_Value_Style_Scale_Offset_Array.Empty();

				Subject.SetFlag(HasPoppingTextFlag, false);

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 池写入 | Write Pooling Info
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("WritePoolingInfo");

		auto Chain = Mechanism->EnchainSolid(RenderBatchFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

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

	// 发送至Niagara | Send Data to Niagara
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("SendDataToNiagara");

		Mechanism->Operate<FUnsafeChain>(RenderBatchFilter,
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


	//---------------------其它游戏线程逻辑 | Other Game Thread Logic--------------------

	// 生成Actor | Spawn Actors
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("SpawnActors");

		Mechanism->Operate<FUnsafeChain>(SpawnActorsFilter,
			[&](FSubjectHandle Subject,
				FActorSpawnConfig& Config)
			{
				if (Config.Delay <= 0)
				{
					if (Config.bEnable && Config.Quantity > 0 && Config.ActorClass)
					{
						FActorSpawnParameters SpawnParams;
						SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

						for (int32 i = 0; i < Config.Quantity; ++i)
						{
							AActor* Actor = CurrentWorld->SpawnActor<AActor>(Config.ActorClass, Config.Transform, SpawnParams);

							if (Actor != nullptr)
							{
								Actor->SetActorScale3D(Config.Transform.GetScale3D());
							}
						}
					}
					Subject.Despawn();
				}
				else
				{
					Config.Delay -= SafeDeltaTime;
				}
			});
	}
	#pragma endregion

	// 生成粒子特效 | Spawn Fx
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("SpawnFx");

		Mechanism->Operate<FUnsafeChain>(SpawnFxFilter,
			[&](FSubjectHandle Subject,
				FFxConfig& Config)
			{
				if (Config.Delay <= 0)
				{
					// 验证合批情况下的SubType
					if (Config.SubType != EESubType::None)
					{
						FLocated FxLocated = { Config.Transform.GetLocation() };
						FDirected FxDirected = { Config.Transform.GetRotation().GetForwardVector() };
						FScaled FxScaled = { Config.Transform.GetScale3D() };

						FSubjectRecord FxRecord;
						FxRecord.SetTrait(FSpawningFx{});
						FxRecord.SetTrait(FxLocated);
						FxRecord.SetTrait(FxDirected);
						FxRecord.SetTrait(FxScaled);

						UBattleFrameFunctionLibraryRT::SetRecordSubTypeTraitByEnum(Config.SubType, FxRecord);

						for (int32 i = 0; i < Config.Quantity; ++i)
						{
							Mechanism->SpawnSubjectDeferred(FxRecord);
						}
					}

					// 处理非合批情况
					if (Config.NiagaraAsset || Config.CascadeAsset)
					{
						for (int32 i = 0; i < Config.Quantity; ++i)
						{
							if (Config.NiagaraAsset)
							{
								UNiagaraFunctionLibrary::SpawnSystemAtLocation(
									CurrentWorld,
									Config.NiagaraAsset,
									Config.Transform.GetLocation(),
									Config.Transform.GetRotation().Rotator(),
									Config.Transform.GetScale3D(),
									true,  // bAutoDestroy
									true,  // bAutoActivate
									ENCPoolMethod::AutoRelease
								);
							}

							if (Config.CascadeAsset)
							{
								UGameplayStatics::SpawnEmitterAtLocation(
									CurrentWorld,
									Config.CascadeAsset,
									Config.Transform.GetLocation(),
									Config.Transform.GetRotation().Rotator(),
									Config.Transform.GetScale3D(),
									true,  // bAutoDestroy
									EPSCPoolMethod::AutoRelease
								);
							}
						}
					}
					Subject.Despawn();
				}
				else
				{
					Config.Delay -= SafeDeltaTime;
				}
			});
	}
	#pragma endregion

	// 播放声音 | Play Sound
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("PlaySound");

		Mechanism->Operate<FUnsafeChain>(PlaySoundFilter,
			[&](FSubjectHandle Subject,
				FSoundConfig& Config)
			{
				if (Config.Delay <= 0)
				{
					if (Config.Sound)
					{
						SoundsToPlay.Enqueue(Config.Sound);
					}
					Subject.Despawn();
				}
				else
				{
					Config.Delay -= SafeDeltaTime;
				}
			});

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

			StreamableManager.RequestAsyncLoad(Sound.ToSoftObjectPath(), FStreamableDelegate::CreateLambda([this, Sound]()
				{
					// 播放加载完成的音效
					UGameplayStatics::PlaySound2D(GetWorld(), Sound.Get(), SoundVolume);
				}));
		}
	}
	#pragma endregion

	// 伤害结果蓝图接口 | DmgResult Interface
	#pragma region
	{
		while (!DamageResultQueue.IsEmpty())
		{
			FDmgResult DmgResult;
			DamageResultQueue.Dequeue(DmgResult);

			if (DmgResult.DamagedSubject.IsValid())
			{
				AActor* DmgActor = DmgResult.DamagedSubject.GetSubjective()->GetActor();

				// 检查actor是否实现了伤害接口
				if (DmgActor && DmgActor->GetClass()->ImplementsInterface(UDmgResultInterface::StaticClass()))
				{
					// 调用接口方法
					IDmgResultInterface::Execute_ReceiveDamage(DmgActor, DmgResult);
				}
			}
		}
	}
	#pragma endregion

	// WIP Draw Debug Shapes

	// WIP Print Log	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ABattleFrameBattleControl::ApplyDamageToSubjects(const FSubjectArray& Subjects, const FSubjectArray& IgnoreSubjects, const FSubjectHandle DmgInstigator, const FVector& HitFromLocation, const FDmgSphere& DmgSphere, const FDebuff& Debuff, TArray<FDmgResult>& DamageResults)
{
	// Record for deferred spawning of TemporalDamager
	FTemporalDamaging TemporalDamaging;

	// 使用TSet存储唯一敌人句柄
	TSet<FSubjectHandle> UniqueHandles;

	// 将IgnoreSubjects转换为TSet以提高查找效率
	const TSet<FSubjectHandle> IgnoreSet(IgnoreSubjects.Subjects);

	for (const auto& Overlapper : Subjects.Subjects)
	{
		// 使用TSet的Contains替代数组的Contains
		if (IgnoreSet.Contains(Overlapper)) continue;

		int32 previousNum = UniqueHandles.Num();
		UniqueHandles.Add(Overlapper);

		if (UniqueHandles.Num() == previousNum) continue;

		// Pre-calculate all trait checks
		const bool bHasHealth = Overlapper.HasTrait<FHealth>();
		const bool bHasLocated = Overlapper.HasTrait<FLocated>();
		const bool bHasDirected = Overlapper.HasTrait<FDirected>();
		const bool bHasCollider = Overlapper.HasTrait<FCollider>();
		const bool bHasDefence = Overlapper.HasTrait<FDefence>();
		const bool bHasTextPopUp = Overlapper.HasTrait<FTextPopUp>();
		const bool bHasMove = Overlapper.HasTrait<FMove>();
		const bool bHasMoving = Overlapper.HasTrait<FMoving>();
		const bool bHasFreezing = Overlapper.HasTrait<FFreezing>();
		const bool bHasBurning = Overlapper.HasTrait<FBurning>();
		const bool bHasAnimation = Overlapper.HasTrait<FAnimation>();
		const bool bHasSleep = Overlapper.HasTrait<FSleep>();
		const bool bHasSleeping = Overlapper.HasTrait<FSleeping>();
		const bool bHasHit = Overlapper.HasTrait<FHit>();
		const bool bHasHitGlow = Overlapper.HasTrait<FHitGlow>();
		const bool bHasJiggle = Overlapper.HasTrait<FJiggle>();
		const bool bHasPatrolling = Overlapper.HasTrait<FPatrolling>();
		const bool bHasTrace = Overlapper.HasTrait<FTrace>();
		const bool bHasIsSubjective = Overlapper.HasTrait<FIsSubjective>();

		FVector Location = bHasLocated ? Overlapper.GetTrait<FLocated>().Location : FVector::ZeroVector;
		FVector Direction = bHasDirected ? Overlapper.GetTrait<FDirected>().Direction : FVector::ZeroVector;
		
		FDmgResult DmgResult;
		DmgResult.DamagedSubject = Overlapper;

		//-------------伤害和抗性------------

		float NormalDmgMult = 1;
		float KineticDmgMult = 1;
		float FireDmgMult = 1;
		float FireDebuffMult = 1;
		float IceDmgMult = 1;
		float IceDebuffMult = 1;
		float PercentDmgMult = 1;

		if (bHasHealth)
		{
			auto& Health = Overlapper.GetTraitRef<FHealth, EParadigm::Unsafe>();

			// 抗性 如果有的话
			if (bHasDefence)
			{
				const auto& Defence = Overlapper.GetTraitRef<FDefence, EParadigm::Unsafe>();

				NormalDmgMult = 1 - Defence.NormalDmgImmune;
				KineticDmgMult = 1 - Defence.KineticDmgImmune;
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
			float PercentageDamage = Health.Maximum * DmgSphere.PercentDmg * PercentDmgMult;

			// 总伤害
			float CombinedDamage = BaseDamage + PercentageDamage;

			// 考虑暴击后伤害
			auto [bIsCrit, PostCritDamage] = ProcessCritDamage(CombinedDamage, DmgSphere.CritMult, DmgSphere.CritProbability);

			// 限制伤害以不大于剩余血量
			float ClampedDamage = FMath::Min(PostCritDamage, Health.Current);

			DmgResult.IsCritical = bIsCrit;
			DmgResult.IsKill = Health.Current == ClampedDamage;
			DmgResult.DmgDealt = ClampedDamage;

			// 应用伤害
			Health.DamageToTake.Enqueue(ClampedDamage);

			// 记录伤害施加者
			if (DmgInstigator.IsValid())
			{
				Health.DamageInstigator.Enqueue(DmgInstigator);
				DmgResult.InstigatorSubject = DmgInstigator;
			}
			else
			{
				Health.DamageInstigator.Enqueue(FSubjectHandle());
			}

			// ------------生成文字--------------

			if (bHasTextPopUp && bHasLocated)
			{
				const auto TextPopUp = Overlapper.GetTrait<FTextPopUp>();

				if (TextPopUp.Enable)
				{
					float Style = 0;
					float Radius = 0.f;

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

					if (bHasCollider)
					{
						Radius = Overlapper.GetTrait<FCollider>().Radius;
					}

					Location += FVector(0, 0, Radius);

					QueueText(FTextPopConfig( Overlapper, PostCritDamage, Style, TextPopUp.TextScale, Radius * 1.1, Location ));
				}
			}

			// 灼烧
			if (Debuff.bCanBurn)
			{
				TemporalDamaging.TotalTemporalDamage = BaseDamage * Debuff.BurnDmgRatio * FireDebuffMult;

				if (TemporalDamaging.TotalTemporalDamage > 0)
				{
					// 让目标材质变红
					if (bHasAnimation)
					{
						auto& TargetAnimation = Overlapper.GetTraitRef<FAnimation, EParadigm::Unsafe>();
						TargetAnimation.Lock();
						TargetAnimation.BurnFx = 1;
						TargetAnimation.Unlock();
					}

					if (bHasBurning)
					{
						auto& TargetBurning = Overlapper.GetTraitRef<FBurning, EParadigm::Unsafe>();
						TargetBurning.Lock();
						Overlapper.GetTraitRef<FBurning, EParadigm::Unsafe>().RemainingTemporalDamager ++;
						TargetBurning.Unlock();
					}
					else
					{
						Overlapper.SetTraitDeferred(FBurning());
					}

					TemporalDamaging.TemporalDamageTarget = Overlapper;
					TemporalDamaging.RemainingTemporalDamage = TemporalDamaging.TotalTemporalDamage;

					if (DmgInstigator.IsValid())
					{
						TemporalDamaging.TemporalDamageInstigator = DmgInstigator;
					}
					else
					{
						TemporalDamaging.TemporalDamageInstigator = FSubjectHandle();
					}

					Mechanism->SpawnSubject(TemporalDamaging);
				}
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
			if (bHasMove && bHasMoving)
			{
				auto& Moving = Overlapper.GetTraitRef<FMoving, EParadigm::Unsafe>();
				const auto& Move = Overlapper.GetTraitRef<FMove, EParadigm::Unsafe>();
				FVector KnockbackForce = FVector(Debuff.KnockbackSpeed.X, Debuff.KnockbackSpeed.X, 1) * HitDirection + FVector(0, 0, Debuff.KnockbackSpeed.Y);
				FVector CombinedForce = Moving.LaunchVelSum + KnockbackForce;

				Moving.Lock();
				Moving.LaunchVelSum += KnockbackForce; // 累加击退力
				Moving.Unlock();
			}
		}

		// 冰冻
		if (Debuff.bCanFreeze)
		{
			if (bHasFreezing)
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
				NewFreezing.FreezeTimeout = Debuff.FreezeTime * (bHasHealth ? IceDebuffMult : 1.0f);
				NewFreezing.FreezeStr = Debuff.FreezeStr * (bHasHealth ? IceDebuffMult : 1.0f);

				if (NewFreezing.FreezeTimeout > 0 && NewFreezing.FreezeStr > 0)
				{
					if (bHasAnimation)
					{
						auto& Animation = Overlapper.GetTraitRef<FAnimation, EParadigm::Unsafe>();
						Animation.PreviousSubjectState = ESubjectState::Dirty; // 强制刷新动画状态机
						Animation.FreezeFx = 1;
					}
					Overlapper.SetTraitDeferred(NewFreezing);
				}
			}
		}

		//-----------其它效果------------

		if (bHasSleeping)// wake on hit
		{
			if (bHasSleep && Overlapper.GetTrait<FSleep>().bWakeOnHit)
			{
				Overlapper.GetTraitRef<FSleep, EParadigm::Unsafe>().bEnable = false;
				Overlapper.RemoveTraitDeferred<FSleeping>();
			}
		}

		if (bHasTrace)
		{
			auto& Trace = Overlapper.GetTraitRef<FTrace, EParadigm::Unsafe>();
			Trace.TraceResult = DmgInstigator;
		}

		if (bHasPatrolling)
		{
			Overlapper.RemoveTraitDeferred<FPatrolling>();
		}

		if (bHasHit)
		{
			const auto& Hit = Overlapper.GetTraitRef<FHit, EParadigm::Unsafe>();

			// Glow
			if (Hit.bCanGlow && !bHasHitGlow)
			{
				Overlapper.SetTraitDeferred(FHitGlow());
			}

			// Jiggle
			if (Hit.JiggleStr != 0.f && !bHasJiggle)
			{
				Overlapper.SetTraitDeferred(FJiggle());
			}

			// Actor
			for (FActorSpawnConfig Config : Hit.SpawnActor)
			{
				Config.SubjectTrans = FTransform(Direction.Rotation(), Location, Config.Transform.GetScale3D());
				QueueActor(Config);
			}

			// Fx
			for (FFxConfig Config : Hit.SpawnFx)
			{
				Config.SubjectTrans = FTransform(HitDirection.Rotation(), Location, Config.Transform.GetScale3D());
				QueueFx(Config);
			}

			// Sound
			for (FSoundConfig Config : Hit.PlaySound)
			{
				Config.SubjectTrans = FTransform(Direction.Rotation(), Location, Config.Transform.GetScale3D());
				QueueSound(Config);
			}
		}
	
		if (bHasIsSubjective)
		{
			DamageResultQueue.Enqueue(DmgResult);
		}

		DamageResults.Add(DmgResult);
	}
}

FVector ABattleFrameBattleControl::FindNewPatrolGoalLocation(const FPatrol& Patrol, const FCollider& Collider, const FTrace& Trace, const FLocated& Located, int32 MaxAttempts)
{
	// Early out if no neighbor grid available
	if (!IsValid(Trace.NeighborGrid))
	{
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const float Distance = FMath::FRandRange(Patrol.PatrolRadiusMin, Patrol.PatrolRadiusMax);
		return Patrol.Origin + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.f);
	}

	FVector BestCandidate = Patrol.Origin;
	float BestDistanceSq = 0.f;

	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		// Generate random position in patrol ring
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const float Distance = FMath::FRandRange(Patrol.PatrolRadiusMin, Patrol.PatrolRadiusMax);
		const FVector Candidate = Patrol.Origin + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.f);

		// Skip visibility check if not required
		if (!Patrol.bCheckVisibility)
		{
			return Candidate;
		}

		// Check visibility through neighbor grid
		bool bHit = false;
		FTraceResult Result;
		Trace.NeighborGrid->SphereSweepForObstacle(Located.Location, Candidate, Collider.Radius, bHit, Result);

		// Return first valid candidate found
		if (!bHit)
		{
			return Candidate;
		}

		// Track farthest candidate as fallback
		const float CurrentDistanceSq = (Candidate - Located.Location).SizeSquared();

		if (CurrentDistanceSq > BestDistanceSq)
		{
			BestCandidate = Candidate;
			BestDistanceSq = CurrentDistanceSq;
		}
	}

	// Return best candidate if all attempts hit obstacles
	return BestCandidate;
}

void ABattleFrameBattleControl::DefineFilters()
{
	// this is a bit inconvenient but good for performance
	bIsFilterReady = true;
	AgentCountFilter = FFilter::Make<FAgent>();
	AgentAgeFilter = FFilter::Make<FStatistics>();
	AgentAppeaFilter = FFilter::Make<FAgent, FRendering, FLocated, FDirected, FAppear, FAppearing, FAnimation, FActivated>();
	AgentAppearAnimFilter = FFilter::Make<FAgent, FRendering, FAnimation, FAppear, FAppearAnim, FActivated>();
	AgentAppearDissolveFilter = FFilter::Make<FAgent, FRendering, FAppearDissolve, FAnimation, FCurves, FActivated>();
	AgentTraceFilter = FFilter::Make<FAgent, FLocated, FDirected, FCollider, FSleep, FPatrol, FTrace, FActivated, FRendering>().Exclude<FAppearing, FAttacking, FDying>();
	AgentAttackFilter = FFilter::Make<FAgent, FAttack, FRendering, FLocated, FDirected, FTrace, FCollider, FActivated>().Exclude<FAppearing, FSleeping, FPatrolling, FAttacking, FDying>();
	AgentAttackingFilter = FFilter::Make<FAgent, FAttack, FRendering, FLocated, FAnimation, FAttacking, FMove, FMoving, FDirected, FTrace, FDebuff, FDamage, FDefence, FActivated>().Exclude<FAppearing, FSleeping, FPatrolling, FDying>();
	AgentHitGlowFilter = FFilter::Make<FAgent, FRendering, FHitGlow, FAnimation, FCurves, FActivated>();
	AgentJiggleFilter = FFilter::Make<FAgent, FRendering, FJiggle, FScaled, FHit, FCurves, FActivated>();
	AgentBurningFilter = FFilter::Make<FTemporalDamaging>();
	AgentFrozenFilter = FFilter::Make<FAgent, FRendering, FAnimation, FFreezing, FActivated>().Exclude<FDying>();
	DecideHealthFilter = FFilter::Make<FHealth, FLocated>().Exclude<FDying>();
	AgentHealthBarFilter = FFilter::Make<FAgent, FRendering, FHealth, FHealthBar, FActivated>();
	AgentDeathFilter = FFilter::Make<FAgent, FRendering, FDeath, FLocated, FDying, FDirected, FTrace, FMove, FMoving, FActivated>();
	AgentDeathDissolveFilter = FFilter::Make<FAgent, FRendering, FDeathDissolve, FAnimation, FDying, FDeath, FCurves, FActivated>();
	AgentDeathAnimFilter = FFilter::Make<FAgent, FRendering, FDeathAnim, FAnimation, FDying, FActivated>();
	SpeedLimitOverrideFilter = FFilter::Make<FCollider, FLocated, FSphereObstacle>();
	AgentPatrolFilter = FFilter::Make<FAgent, FCollider, FLocated, FPatrol, FTrace, FMove, FMoving, FActivated, FRendering>().Exclude<FAppearing, FSleeping, FAttacking, FDying>();
	AgentMoveFilter = FFilter::Make<FAgent, FRendering, FAnimation, FMove, FMoving, FDirected, FLocated, FScaled, FAttack, FTrace, FNavigation, FAvoidance, FAvoiding, FCollider, FDefence, FPatrol, FGridData, FActivated>();
	IdleToMoveAnimFilter = FFilter::Make<FAgent, FAnimation, FMove, FMoving, FDeath, FActivated>().Exclude<FAppearing, FDying>();
	AgentStateMachineFilter = FFilter::Make<FAgent, FAnimation, FRendering, FAppear, FAttack, FDeath, FMoving, FActivated>();
	RenderBatchFilter = FFilter::Make<FRenderBatchData>();
	AgentRenderFilter = FFilter::Make<FAgent, FRendering, FDirected, FScaled, FLocated, FAnimation, FHealth, FHealthBar, FPoppingText, FCollider, FActivated>();
	TextRenderFilter = FFilter::Make<FAgent, FRendering, FTextPopUp, FPoppingText, FActivated>().IncludeFlag(HasPoppingTextFlag);
	SpawnActorsFilter = FFilter::Make<FActorSpawnConfig>();
	SpawnFxFilter = FFilter::Make<FFxConfig>();
	PlaySoundFilter = FFilter::Make<FSoundConfig>();
	SubjectFilterBase = FFilter::Make<FLocated, FCollider, FAvoidance, FAvoiding, FGridData, FActivated>().Exclude<FSphereObstacle, FBoxObstacle, FCorpse>();
}

//-------------------------------RVO2D Copyright 2023, EastFoxStudio. All Rights Reserved-------------------------------

void ABattleFrameBattleControl::ComputeAvoidingVelocity(FAvoidance& Avoidance, FAvoiding& Avoiding, const TArray<FGridData>& SubjectNeighbors, const TArray<FGridData>& ObstacleNeighbors, float TimeStep_)
{
	Avoidance.OrcaLines.clear();

	/* Create obstacle ORCA lines. */
	if (!ObstacleNeighbors.IsEmpty())
	{
		const float invTimeHorizonObst = 1.0f / Avoidance.RVO_TimeHorizon_Obstacle;

		for (const auto& Data : ObstacleNeighbors)
		{
			FBoxObstacle* obstacle1 = Data.SubjectHandle.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();
			FBoxObstacle* obstacle2 = obstacle1->nextObstacle_.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();

			const RVO::Vector2 relativePosition1 = obstacle1->point_ - Avoiding.Position;
			const RVO::Vector2 relativePosition2 = obstacle2->point_ - Avoiding.Position;

			/*
			 * Check if velocity obstacle of obstacle is already taken care of by
			 * previously constructed obstacle ORCA lines.
			 */
			bool alreadyCovered = false;

			for (size_t j = 0; j < Avoidance.OrcaLines.size(); ++j) {
				if (RVO::det(invTimeHorizonObst * relativePosition1 - Avoidance.OrcaLines[j].point, Avoidance.OrcaLines[j].direction) - invTimeHorizonObst * Avoiding.Radius >= -RVO_EPSILON && det(invTimeHorizonObst * relativePosition2 - Avoidance.OrcaLines[j].point, Avoidance.OrcaLines[j].direction) - invTimeHorizonObst * Avoiding.Radius >= -RVO_EPSILON) {
					alreadyCovered = true;
					break;
				}
			}

			if (alreadyCovered) {
				continue;
			}

			/* Not yet covered. Check for collisions. */

			const float distSq1 = RVO::absSq(relativePosition1);
			const float distSq2 = RVO::absSq(relativePosition2);

			const float radiusSq = RVO::sqr(Avoiding.Radius);

			const RVO::Vector2 obstacleVector = obstacle2->point_ - obstacle1->point_;
			const float s = (-relativePosition1 * obstacleVector) / absSq(obstacleVector);
			const float distSqLine = absSq(-relativePosition1 - s * obstacleVector);

			RVO::Line line;

			if (s < 0.0f && distSq1 <= radiusSq) {
				/* Collision with left vertex. Ignore if non-convex. */
				if (obstacle1->isConvex_) {
					line.point = RVO::Vector2(0.0f, 0.0f);
					line.direction = normalize(RVO::Vector2(-relativePosition1.y(), relativePosition1.x()));
					Avoidance.OrcaLines.push_back(line);
				}
				continue;
			}
			else if (s > 1.0f && distSq2 <= radiusSq) {
				/* Collision with right vertex. Ignore if non-convex
				 * or if it will be taken care of by neighoring obstace */
				if (obstacle2->isConvex_ && det(relativePosition2, obstacle2->unitDir_) >= 0.0f) {
					line.point = RVO::Vector2(0.0f, 0.0f);
					line.direction = normalize(RVO::Vector2(-relativePosition2.y(), relativePosition2.x()));
					Avoidance.OrcaLines.push_back(line);
				}
				continue;
			}
			else if (s >= 0.0f && s < 1.0f && distSqLine <= radiusSq) {
				/* Collision with obstacle segment. */
				line.point = RVO::Vector2(0.0f, 0.0f);
				line.direction = -obstacle1->unitDir_;
				Avoidance.OrcaLines.push_back(line);
				continue;
			}

			/*
			 * No collision.
			 * Compute legs. When obliquely viewed, both legs can come from a single
			 * vertex. Legs extend cut-off line when nonconvex vertex.
			 */

			RVO::Vector2 leftLegDirection, rightLegDirection;

			if (s < 0.0f && distSqLine <= radiusSq) {
				/*
				 * Obstacle viewed obliquely so that left vertex
				 * defines velocity obstacle.
				 */
				if (!obstacle1->isConvex_) {
					/* Ignore obstacle. */
					continue;
				}

				obstacle2 = obstacle1;

				const float leg1 = std::sqrt(distSq1 - radiusSq);
				leftLegDirection = RVO::Vector2(relativePosition1.x() * leg1 - relativePosition1.y() * Avoiding.Radius, relativePosition1.x() * Avoiding.Radius + relativePosition1.y() * leg1) / distSq1;
				rightLegDirection = RVO::Vector2(relativePosition1.x() * leg1 + relativePosition1.y() * Avoiding.Radius, -relativePosition1.x() * Avoiding.Radius + relativePosition1.y() * leg1) / distSq1;
			}
			else if (s > 1.0f && distSqLine <= radiusSq) {
				/*
				 * Obstacle viewed obliquely so that
				 * right vertex defines velocity obstacle.
				 */
				if (!obstacle2->isConvex_) {
					/* Ignore obstacle. */
					continue;
				}

				obstacle1 = obstacle2;

				const float leg2 = std::sqrt(distSq2 - radiusSq);
				leftLegDirection = RVO::Vector2(relativePosition2.x() * leg2 - relativePosition2.y() * Avoiding.Radius, relativePosition2.x() * Avoiding.Radius + relativePosition2.y() * leg2) / distSq2;
				rightLegDirection = RVO::Vector2(relativePosition2.x() * leg2 + relativePosition2.y() * Avoiding.Radius, -relativePosition2.x() * Avoiding.Radius + relativePosition2.y() * leg2) / distSq2;
			}
			else {
				/* Usual situation. */
				if (obstacle1->isConvex_) {
					const float leg1 = std::sqrt(distSq1 - radiusSq);
					leftLegDirection = RVO::Vector2(relativePosition1.x() * leg1 - relativePosition1.y() * Avoiding.Radius, relativePosition1.x() * Avoiding.Radius + relativePosition1.y() * leg1) / distSq1;
				}
				else {
					/* Left vertex non-convex; left leg extends cut-off line. */
					leftLegDirection = -obstacle1->unitDir_;
				}

				if (obstacle2->isConvex_) {
					const float leg2 = std::sqrt(distSq2 - radiusSq);
					rightLegDirection = RVO::Vector2(relativePosition2.x() * leg2 + relativePosition2.y() * Avoiding.Radius, -relativePosition2.x() * Avoiding.Radius + relativePosition2.y() * leg2) / distSq2;
				}
				else {
					/* Right vertex non-convex; right leg extends cut-off line. */
					rightLegDirection = obstacle1->unitDir_;
				}
			}

			/*
			 * Legs can never point into neighboring edge when convex vertex,
			 * take cutoff-line of neighboring edge instead. If velocity projected on
			 * "foreign" leg, no constraint is added.
			 */

			const FSubjectHandle leftNeighbor = obstacle1->prevObstacle_;

			bool isLeftLegForeign = false;
			bool isRightLegForeign = false;

			if (obstacle1->isConvex_ && det(leftLegDirection, -obstacle1->unitDir_) >= 0.0f) {
				/* Left leg points into obstacle. */
				leftLegDirection = -obstacle1->unitDir_;
				isLeftLegForeign = true;
			}

			if (obstacle2->isConvex_ && det(rightLegDirection, obstacle2->unitDir_) <= 0.0f) {
				/* Right leg points into obstacle. */
				rightLegDirection = obstacle2->unitDir_;
				isRightLegForeign = true;
			}

			/* Compute cut-off centers. */
			const RVO::Vector2 leftCutoff = invTimeHorizonObst * (obstacle1->point_ - Avoiding.Position);
			const RVO::Vector2 rightCutoff = invTimeHorizonObst * (obstacle2->point_ - Avoiding.Position);
			const RVO::Vector2 cutoffVec = rightCutoff - leftCutoff;

			/* Project current velocity on velocity obstacle. */

			/* Check if current velocity is projected on cutoff circles. */
			const float t = (obstacle1 == obstacle2 ? 0.5f : ((Avoiding.CurrentVelocity - leftCutoff) * cutoffVec) / absSq(cutoffVec));
			const float tLeft = ((Avoiding.CurrentVelocity - leftCutoff) * leftLegDirection);
			const float tRight = ((Avoiding.CurrentVelocity - rightCutoff) * rightLegDirection);

			if ((t < 0.0f && tLeft < 0.0f) || (obstacle1 == obstacle2 && tLeft < 0.0f && tRight < 0.0f)) {
				/* Project on left cut-off circle. */
				const RVO::Vector2 unitW = normalize(Avoiding.CurrentVelocity - leftCutoff);

				line.direction = RVO::Vector2(unitW.y(), -unitW.x());
				line.point = leftCutoff + Avoiding.Radius * invTimeHorizonObst * unitW;
				Avoidance.OrcaLines.push_back(line);
				continue;
			}
			else if (t > 1.0f && tRight < 0.0f) {
				/* Project on right cut-off circle. */
				const RVO::Vector2 unitW = normalize(Avoiding.CurrentVelocity - rightCutoff);

				line.direction = RVO::Vector2(unitW.y(), -unitW.x());
				line.point = rightCutoff + Avoiding.Radius * invTimeHorizonObst * unitW;
				Avoidance.OrcaLines.push_back(line);
				continue;
			}

			/*
			 * Project on left leg, right leg, or cut-off line, whichever is closest
			 * to velocity.
			 */
			const float distSqCutoff = ((t < 0.0f || t > 1.0f || obstacle1 == obstacle2) ? std::numeric_limits<float>::infinity() : absSq(Avoiding.CurrentVelocity - (leftCutoff + t * cutoffVec)));
			const float distSqLeft = ((tLeft < 0.0f) ? std::numeric_limits<float>::infinity() : absSq(Avoiding.CurrentVelocity - (leftCutoff + tLeft * leftLegDirection)));
			const float distSqRight = ((tRight < 0.0f) ? std::numeric_limits<float>::infinity() : absSq(Avoiding.CurrentVelocity - (rightCutoff + tRight * rightLegDirection)));

			if (distSqCutoff <= distSqLeft && distSqCutoff <= distSqRight) {
				/* Project on cut-off line. */
				line.direction = -obstacle1->unitDir_;
				line.point = leftCutoff + Avoiding.Radius * invTimeHorizonObst * RVO::Vector2(-line.direction.y(), line.direction.x());
				Avoidance.OrcaLines.push_back(line);
				continue;
			}
			else if (distSqLeft <= distSqRight) {
				/* Project on left leg. */
				if (isLeftLegForeign) {
					continue;
				}

				line.direction = leftLegDirection;
				line.point = leftCutoff + Avoiding.Radius * invTimeHorizonObst * RVO::Vector2(-line.direction.y(), line.direction.x());
				Avoidance.OrcaLines.push_back(line);
				continue;
			}
			else {
				/* Project on right leg. */
				if (isRightLegForeign) {
					continue;
				}

				line.direction = -rightLegDirection;
				line.point = rightCutoff + Avoiding.Radius * invTimeHorizonObst * RVO::Vector2(-line.direction.y(), line.direction.x());
				Avoidance.OrcaLines.push_back(line);
				continue;
			}
		}
	}

	const size_t numObstLines = Avoidance.OrcaLines.size();

	/* Create agent ORCA lines. */
	if (!SubjectNeighbors.IsEmpty())
	{
		const float invTimeHorizon = 1.0f / Avoidance.RVO_TimeHorizon_Agent;

		for (const auto& Data : SubjectNeighbors)
		{
			const auto& OtherAvoiding = Data.SubjectHandle.GetTraitRef<FAvoiding, EParadigm::Unsafe>();
			const RVO::Vector2 relativePosition = OtherAvoiding.Position - Avoiding.Position;
			const RVO::Vector2 relativeVelocity = Avoiding.CurrentVelocity - OtherAvoiding.CurrentVelocity;
			const float distSq = absSq(relativePosition);
			const float combinedRadius = Avoiding.Radius + OtherAvoiding.Radius;
			const float combinedRadiusSq = RVO::sqr(combinedRadius);

			RVO::Line line;
			RVO::Vector2 u;

			if (distSq > combinedRadiusSq) {
				/* No collision. */
				const RVO::Vector2 w = relativeVelocity - invTimeHorizon * relativePosition;
				/* Vector from cutoff center to relative velocity. */
				const float wLengthSq = RVO::absSq(w);

				const float dotProduct1 = w * relativePosition;

				if (dotProduct1 < 0.0f && RVO::sqr(dotProduct1) > combinedRadiusSq * wLengthSq) {
					/* Project on cut-off circle. */
					const float wLength = std::sqrt(wLengthSq);
					const RVO::Vector2 unitW = w / wLength;

					line.direction = RVO::Vector2(unitW.y(), -unitW.x());
					u = (combinedRadius * invTimeHorizon - wLength) * unitW;
				}
				else {
					/* Project on legs. */
					const float leg = std::sqrt(distSq - combinedRadiusSq);

					if (det(relativePosition, w) > 0.0f) {
						/* Project on left leg. */
						line.direction = RVO::Vector2(relativePosition.x() * leg - relativePosition.y() * combinedRadius, relativePosition.x() * combinedRadius + relativePosition.y() * leg) / distSq;
					}
					else {
						/* Project on right leg. */
						line.direction = -RVO::Vector2(relativePosition.x() * leg + relativePosition.y() * combinedRadius, -relativePosition.x() * combinedRadius + relativePosition.y() * leg) / distSq;
					}

					const float dotProduct2 = relativeVelocity * line.direction;

					u = dotProduct2 * line.direction - relativeVelocity;
				}
			}
			else {
				/* Collision. Project on cut-off circle of time timeStep. */
				const float invTimeStep = 1.0f / TimeStep_;

				/* Vector from cutoff center to relative velocity. */
				const RVO::Vector2 w = relativeVelocity - invTimeStep * relativePosition;

				const float wLength = abs(w);
				const RVO::Vector2 unitW = w / wLength;

				line.direction = RVO::Vector2(unitW.y(), -unitW.x());
				u = (combinedRadius * invTimeStep - wLength) * unitW;
			}

			float Ratio = OtherAvoiding.bCanAvoid ? 0.5f : 1.f;
			line.point = Avoiding.CurrentVelocity + Ratio * u;
			Avoidance.OrcaLines.push_back(line);
		}
	}

	size_t lineFail = LinearProgram2(Avoidance.OrcaLines, Avoidance.MaxSpeed, Avoidance.DesiredVelocity, false, Avoidance.AvoidingVelocity);

	if (lineFail < Avoidance.OrcaLines.size()) {
		LinearProgram3(Avoidance.OrcaLines, numObstLines, lineFail, Avoidance.MaxSpeed, Avoidance.AvoidingVelocity);
	}
}

bool ABattleFrameBattleControl::LinearProgram1(const std::vector<RVO::Line>& lines, size_t lineNo, float radius, const RVO::Vector2& optVelocity, bool directionOpt, RVO::Vector2& result)
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("linearProgram1");
	const float dotProduct = lines[lineNo].point * lines[lineNo].direction;
	const float discriminant = RVO::sqr(dotProduct) + RVO::sqr(radius) - absSq(lines[lineNo].point);

	if (discriminant < 0.0f) {
		/* Max speed circle fully invalidates line lineNo. */
		return false;
	}

	const float sqrtDiscriminant = std::sqrt(discriminant);
	float tLeft = -dotProduct - sqrtDiscriminant;
	float tRight = -dotProduct + sqrtDiscriminant;

	for (size_t i = 0; i < lineNo; ++i) {
		const float denominator = det(lines[lineNo].direction, lines[i].direction);
		const float numerator = det(lines[i].direction, lines[lineNo].point - lines[i].point);

		if (std::fabs(denominator) <= RVO_EPSILON) {
			/* Lines lineNo and i are (almost) parallel. */
			if (numerator < 0.0f) {
				return false;
			}
			else {
				continue;
			}
		}

		const float t = numerator / denominator;

		if (denominator >= 0.0f) {
			/* Line i bounds line lineNo on the right. */
			tRight = std::min(tRight, t);
		}
		else {
			/* Line i bounds line lineNo on the left. */
			tLeft = std::max(tLeft, t);
		}

		if (tLeft > tRight) {
			return false;
		}
	}

	if (directionOpt) {
		/* Optimize direction. */
		if (optVelocity * lines[lineNo].direction > 0.0f) {
			/* Take right extreme. */
			result = lines[lineNo].point + tRight * lines[lineNo].direction;
		}
		else {
			/* Take left extreme. */
			result = lines[lineNo].point + tLeft * lines[lineNo].direction;
		}
	}
	else {
		/* Optimize closest point. */
		const float t = lines[lineNo].direction * (optVelocity - lines[lineNo].point);

		if (t < tLeft) {
			result = lines[lineNo].point + tLeft * lines[lineNo].direction;
		}
		else if (t > tRight) {
			result = lines[lineNo].point + tRight * lines[lineNo].direction;
		}
		else {
			result = lines[lineNo].point + t * lines[lineNo].direction;
		}
	}

	return true;
}

size_t ABattleFrameBattleControl::LinearProgram2(const std::vector<RVO::Line>& lines, float radius, const RVO::Vector2& optVelocity, bool directionOpt, RVO::Vector2& result)
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("linearProgram2");
	if (directionOpt) {
		/*
		 * Optimize direction. Note that the optimization velocity is of unit
		 * length in this case.
		 */
		result = optVelocity * radius;
	}
	else if (RVO::absSq(optVelocity) > RVO::sqr(radius)) {
		/* Optimize closest point and outside circle. */
		result = normalize(optVelocity) * radius;
	}
	else {
		/* Optimize closest point and inside circle. */
		result = optVelocity;
	}

	for (size_t i = 0; i < lines.size(); ++i) {
		if (det(lines[i].direction, lines[i].point - result) > 0.0f) {
			/* Result does not satisfy constraint i. Compute new optimal result. */
			const RVO::Vector2 tempResult = result;

			if (!LinearProgram1(lines, i, radius, optVelocity, directionOpt, result)) {
				result = tempResult;
				return i;
			}
		}
	}

	return lines.size();
}

void ABattleFrameBattleControl::LinearProgram3(const std::vector<RVO::Line>& lines, size_t numObstLines, size_t beginLine, float radius, RVO::Vector2& result)
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("linearProgram3");
	float distance = 0.0f;

	for (size_t i = beginLine; i < lines.size(); ++i) {
		if (det(lines[i].direction, lines[i].point - result) > distance) {
			/* Result does not satisfy constraint of line i. */
			std::vector<RVO::Line> projLines(lines.begin(), lines.begin() + static_cast<ptrdiff_t>(numObstLines));

			for (size_t j = numObstLines; j < i; ++j) {
				RVO::Line line;

				float determinant = det(lines[i].direction, lines[j].direction);

				if (std::fabs(determinant) <= RVO_EPSILON) {
					/* Line i and line j are parallel. */
					if (lines[i].direction * lines[j].direction > 0.0f) {
						/* Line i and line j point in the same direction. */
						continue;
					}
					else {
						/* Line i and line j point in opposite direction. */
						line.point = 0.5f * (lines[i].point + lines[j].point);
					}
				}
				else {
					line.point = lines[i].point + (det(lines[j].direction, lines[i].point - lines[j].point) / determinant) * lines[i].direction;
				}

				line.direction = normalize(lines[j].direction - lines[i].direction);
				projLines.push_back(line);
			}

			const RVO::Vector2 tempResult = result;

			if (LinearProgram2(projLines, radius, RVO::Vector2(-lines[i].direction.y(), lines[i].direction.x()), true, result) < projLines.size()) {
				/* This should in principle not happen.  The result is by definition
				 * already in the feasible region of this linear program. If it fails,
				 * it is due to small floating point error, and the current result is
				 * kept.
				 */
				result = tempResult;
			}

			distance = det(lines[i].direction, lines[i].point - result);
		}
	}
}
