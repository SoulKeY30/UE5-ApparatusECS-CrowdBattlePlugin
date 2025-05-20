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
					Moving.LaunchForce = FVector::ZeroVector; // 击退力清零
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
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("DecideDamage");

		auto Chain = Mechanism->EnchainSolid(DecideDamageFilter);// it processes hero and prop type too
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FHealth& Health,
				FLocated& Located)
			{
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
			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 更新血条 | Update HealthBar
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentHealthBar");

		auto Chain = Mechanism->EnchainSolid(AgentHealthBarFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FHealth Health,
				FHealthBar& HealthBar)
			{
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
				else if (Moving.CurrentVelocity.Size2D() < Move.MinMoveSpeed && Death.bDisableCollision && !Subject.HasTrait<FCorpse>())
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
					else if (Distance > CombinedRadius * 1.25f)
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
				FMove& Move)
			{
				if (!Patrol.bEnable) return;

				// 巡逻逻辑
				bool HasPatrolling = Subject.HasTrait<FPatrolling>();

				if (HasPatrolling)
				{
					auto& Patrolling = Subject.GetTraitRef<FPatrolling, EParadigm::Unsafe>();

					// 检查是否到达目标点
					const bool bHasArrived = FVector::Dist2D(Located.Location, Move.Goal) < Patrol.AcceptanceRadius;

					if (bHasArrived)
					{
						// 到达目标点后的等待逻辑
						if (Patrolling.WaitTimeLeft <= 0.f)
						{
							ResetPatrol(Patrol, Patrolling, Located);
							Move.Goal = FindNewPatrolGoalLocation(Patrol, Collider, Trace, Located, 3);
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
							Move.Goal = FindNewPatrolGoalLocation(Patrol, Collider, Trace, Located, 3);
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

		FFilter Filter1 = FFilter::Make<FNavigation>();
		auto Chain1 = Mechanism->EnchainSolid(Filter1);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain1->IterableNum(), MaxThreadsAllowed, 1, ThreadsCount, BatchSize);

		// filter out those need reload and mark them
		Chain1->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FNavigation& Navigation)
			{
				Subject.SetFlag(ReloadFlowFieldFlag, false);

				if (UNLIKELY(Navigation.FlowFieldToUse != Navigation.PreviousFlowFieldToUse))
				{
					Navigation.PreviousFlowFieldToUse = Navigation.FlowFieldToUse;

					if (Navigation.FlowFieldToUse.IsValid())
					{
						Subject.SetFlag(ReloadFlowFieldFlag, true);
					}
				}

			}, ThreadsCount, BatchSize);

		// do reload, this only works on game thread
		Mechanism->Operate<FUnsafeChain>(Filter1.IncludeFlag(ReloadFlowFieldFlag),
			[&](FNavigation& Navigation)
			{
				Navigation.FlowField = Navigation.FlowFieldToUse.LoadSynchronous();
			});


		FFilter Filter2 = FFilter::Make<FBindFlowField>();
		auto Chain2 = Mechanism->EnchainSolid(Filter2);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain2->IterableNum(), MaxThreadsAllowed, 1, ThreadsCount, BatchSize);

		Chain2->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FBindFlowField& BindFlowField)
			{
				Subject.SetFlag(ReloadFlowFieldFlag, false);

				if (UNLIKELY(BindFlowField.FlowFieldToBind != BindFlowField.PreviousFlowFieldToBind))
				{
					BindFlowField.PreviousFlowFieldToBind = BindFlowField.FlowFieldToBind;

					if (BindFlowField.FlowFieldToBind.IsValid())
					{
						Subject.SetFlag(ReloadFlowFieldFlag, true);
					}
				}

			}, ThreadsCount, BatchSize);


		Mechanism->Operate<FUnsafeChain>(Filter2.IncludeFlag(ReloadFlowFieldFlag),
			[&](FBindFlowField& BindFlowField)
			{
				BindFlowField.FlowField = BindFlowField.FlowFieldToBind.LoadSynchronous();
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
				FAnimation& Animation,
				FMove& Move,
				FMoving& Moving,
				FDirected& Directed,
				FLocated& Located,
				FAttack& Attack,
				FTrace& Trace,
				FNavigation& Navigation,
				FAvoidance& Avoidance,
				FCollider& Collider,
				FDefence& Defence,
				FPatrol& Patrol)
			{
				//if (!Move.bEnable) return;

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
				Moving.ActiveSpeedMult = 1;
				Moving.PassiveSpeedMult = 1;

				// 默认流场, 必须获取因为之后要用到地面高度
				FCellStruct Cell_BaseFF = FCellStruct{};
				bool bInside_BaseFF = Navigation.FlowField->GetCellAtLocation(AgentLocation, Cell_BaseFF);


				//----------------------------- 寻路 ----------------------------//

				if (Move.bEnable && !bIsAppearing)// Appearing不寻路
				{
					if (bIsPatrolling)// Patrolling直接向目标点移动，之后要做个体寻路
					{
						DesiredMoveDirection = (Move.Goal - AgentLocation).GetSafeNormal2D();
					}
					else
					{
						if (!bIsValidTraceResult) // 无攻击目标
						{
							if (bInside_BaseFF)
							{
								Move.Goal = Navigation.FlowField->goalLocation;
								DesiredMoveDirection = Cell_BaseFF.dir.GetSafeNormal2D();
							}
							else
							{
								Moving.PassiveSpeedMult = 0;
							}
						}
						else
						{
							// 直接向攻击目标移动，之后要做个体寻路
							auto ApproachTraceResultDirectly = [&]()
							{
								if (bIsTraceResultHasLocated)
								{
									Move.Goal = Trace.TraceResult.GetTrait<FLocated>().Location;
									DesiredMoveDirection = (Move.Goal - AgentLocation).GetSafeNormal2D();
								}
								else
								{
									Moving.ActiveSpeedMult = 0;
								}
							};

							if (bIsTraceResultHasLocated)
							{
								Move.Goal = Trace.TraceResult.GetTrait<FLocated>().Location;
							}

							if (bIsTraceResultHasBindFlowField)
							{
								AFlowField* BindFlowField = Trace.TraceResult.GetTrait<FBindFlowField>().FlowField;

								if (IsValid(BindFlowField)) // 从目标获取指向目标的流场
								{
									FCellStruct Cell_TargetFF = FCellStruct{};
									bool bInside_TargetFF = BindFlowField->GetCellAtLocation(AgentLocation, Cell_TargetFF);

									if (bInside_TargetFF)
									{
										Move.Goal = BindFlowField->goalLocation;
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
				

				//--------------------------- 计算水平速度 ----------------------------//

				// Launch Velocity
				if (Moving.bLaunching)
				{
					Moving.CurrentVelocity += Moving.LaunchForce * (1 - (bIsDying ? Defence.KineticDebuffImmuneDead : Defence.KineticDebuffImmuneAlive));

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
				}

				// Speed
				float DistanceToGoal = FVector::Dist2D(AgentLocation, Move.Goal);

				if (!Move.bEnable || bIsAppearing || bIsSleeping || bIsDying)
				{
					Moving.ActiveSpeedMult = 0.0f; // 不移动
				}
				
				if (bIsPatrolling)// 抵达巡逻目标点后停止移动
				{
					if (DistanceToGoal <= Patrol.AcceptanceRadius)
					{
						Moving.ActiveSpeedMult = 0;
					}
					else
					{
						Moving.ActiveSpeedMult *= Patrol.MoveSpeedMult;
					}
				}
				
				if (bIsAttacking)
				{
					EAttackState State = Subject.GetTraitRef<FAttacking, EParadigm::Unsafe>().State;

					if (State != EAttackState::Cooling)
					{
						Moving.PassiveSpeedMult = 0.0f; // 强制停止
					}
				}
				
				if (bIsFreezing)
				{
					const auto& Freezing = Subject.GetTraitRef<FFreezing, EParadigm::Unsafe>();
					Moving.PassiveSpeedMult *= 1.0f - Freezing.FreezeStr; // 强制减速
				}

				// Speed-Angle Curve
				const float DotProduct = FVector::DotProduct(Directed.Direction, DesiredMoveDirection);
				const float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

				const TRange<float> TurnInputRange(Move.TurnSpeedRangeMap.X, Move.TurnSpeedRangeMap.Z);
				const TRange<float> TurnOutputRange(Move.TurnSpeedRangeMap.Y, Move.TurnSpeedRangeMap.W);
				Moving.ActiveSpeedMult *= FMath::GetMappedRangeValueClamped(TurnInputRange, TurnOutputRange, AngleDegrees);

				// Speed-Distance Curve
				if (!bIsPatrolling)
				{
					float OtherRadius = Trace.TraceResult.HasTrait<FCollider>() ? Trace.TraceResult.GetTrait<FCollider>().Radius : 0;
					const float DistanceToTarget = FMath::Clamp(DistanceToGoal - OtherRadius, 0, FLT_MAX);

					if (DistanceToTarget <= Move.AcceptanceRadius)
					{
						Moving.PassiveSpeedMult = 0;
					}
					else
					{
						const TRange<float> MoveInputRange(Move.MoveSpeedRangeMap.X, Move.MoveSpeedRangeMap.Z);
						const TRange<float> MoveOutputRange(Move.MoveSpeedRangeMap.Y, Move.MoveSpeedRangeMap.W);

						Moving.PassiveSpeedMult *= FMath::GetMappedRangeValueClamped(MoveInputRange, MoveOutputRange, DistanceToTarget);
					}
				}

				// Desired Speed
				float DesiredSpeed = Move.MoveSpeed * Moving.ActiveSpeedMult * Moving.PassiveSpeedMult;

				if (DesiredSpeed < Move.MinMoveSpeed)
				{
					DesiredSpeed = 0;
				}

				// Steering
				float SlowMult = 1;

				if (!Move.bEnable || bIsAppearing || bIsSleeping || Moving.bFalling || Moving.bLaunching || bIsDying || (bIsPatrolling && DistanceToGoal <= Patrol.AcceptanceRadius) || (bIsAttacking/* && Subject.GetTrait<FAttacking>().State == EAttackState::PreCast*/))
				{
					SlowMult = 0;// 不转向
				}

				if (bIsFreezing)
				{
					const auto& Freezing = Subject.GetTraitRef<FFreezing, EParadigm::Unsafe>();
					SlowMult *= 1.0f - Freezing.FreezeStr; // 冰冻时转向更慢
				}

				Directed.DesiredRot = DesiredMoveDirection.GetSafeNormal2D().ToOrientationRotator();
					
				const FRotator CurrentRot = Directed.Direction.ToOrientationRotator();
				const FRotator InterpolatedRot = FMath::RInterpTo(CurrentRot, Directed.DesiredRot, SafeDeltaTime, FMath::Clamp(Move.TurnSpeed * SlowMult, 0.0001, FLT_MAX));

				Directed.Direction = InterpolatedRot.Vector();

				// Desired Velocity
				FVector DesiredVelocity = DesiredSpeed * Directed.Direction;
				Moving.DesiredVelocity = FVector(DesiredVelocity.X, DesiredVelocity.Y, 0);

				//---------------------------计算垂直速度-------------------------//

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

						FCellStruct Cell = FCellStruct{};
						bool bInside = Navigation.FlowField->GetCellAtLocation(BoundSamplePoint, Cell);

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
									FVector BounceDecay = FVector(Move.BounceVelocityDecay.X, Move.BounceVelocityDecay.X, Move.BounceVelocityDecay.Y);
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

				//---------------------------- 物理效果 --------------------------//

				if (!Moving.bFalling && Moving.LaunchTimer > 0)// 地面摩擦力
				{
					FVector XYDir = Moving.CurrentVelocity.GetSafeNormal2D();
					float XYSpeed = Moving.CurrentVelocity.Size2D();
					XYSpeed = FMath::Clamp(XYSpeed - Move.Deceleration_Ground * SafeDeltaTime, 0, XYSpeed);
					Moving.CurrentVelocity = XYDir * XYSpeed + FVector(0, 0, Moving.CurrentVelocity.Z);
					XYSpeed < 10.f ? Moving.LaunchTimer = 0 : Moving.LaunchTimer -= SafeDeltaTime;
				}
				else if (Moving.bFalling && !Move.bCanFly)// 空气阻力
				{
					FVector XYDir = Moving.CurrentVelocity.GetSafeNormal();
					float XYSpeed = Moving.CurrentVelocity.Size();
					XYSpeed = FMath::Clamp(XYSpeed - Move.Deceleration_Air * SafeDeltaTime, 0, XYSpeed);
					Moving.CurrentVelocity = XYDir * XYSpeed;
				}

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 避障 | Avoidance
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Avoidance");// agent only do avoidance in xy, not z

		for (UNeighborGridComponent* Grid : NeighborGrids)
		{
			if (Grid)
			{
				Grid->Evaluate();
			}
		}
	}
	#pragma endregion

	
	//----------------------- 渲染 | Rendering ------------------------

	// 待机-移动切换 | Idle-Move Switch
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("IdleToMoveAnim");

		auto Chain = Mechanism->EnchainSolid(IdleToMoveAnimFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, 1, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FAnimation& Animation,
				FMove& Move,
				FMoving& Moving,
				FDeath& Death)
			{
				const bool bIsAttacking = Subject.HasTrait<FAttacking>();

				if (bIsAttacking)
				{
					EAttackState State = Subject.GetTraitRef<FAttacking, EParadigm::Unsafe>().State;

					if (State == EAttackState::Cooling)
					{
						const bool IsMoving = Moving.CurrentVelocity.Size2D() > Move.MinMoveSpeed;
						Animation.SubjectState = IsMoving ? ESubjectState::Moving : ESubjectState::Idle;
					}
				}
				else
				{
					const bool IsMoving = Moving.CurrentVelocity.Size2D() > Move.MinMoveSpeed;
					Animation.SubjectState = IsMoving ? ESubjectState::Moving : ESubjectState::Idle;
				}

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

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
				FMoving& Moving)
			{
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

				Data.Unlock();

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("TextRender");

		auto Chain = Mechanism->EnchainSolid(TextRenderFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

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

			//if (DmgResult.IsKill) Overlapper.SetTrait(FDying());

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

					QueueText(Overlapper, PostCritDamage, Style, TextPopUp.TextScale, Radius * 1.1, Location);
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
				FVector CombinedForce = Moving.LaunchForce + KnockbackForce;

				Moving.Lock();
				Moving.LaunchForce += KnockbackForce; // 累加击退力
				Moving.Unlock();

				if (Moving.LaunchForce.Size() > 0.f)
				{
					Moving.bLaunching = true;
				}
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
	DecideDamageFilter = FFilter::Make<FHealth, FLocated>().Exclude<FDying>();
	AgentHealthBarFilter = FFilter::Make<FAgent, FRendering, FHealth, FHealthBar, FActivated>();
	AgentDeathFilter = FFilter::Make<FAgent, FRendering, FDeath, FLocated, FDying, FDirected, FTrace, FMove, FMoving, FActivated>();
	AgentDeathDissolveFilter = FFilter::Make<FAgent, FRendering, FDeathDissolve, FAnimation, FDying, FDeath, FCurves, FActivated>();
	AgentDeathAnimFilter = FFilter::Make<FAgent, FRendering, FDeathAnim, FAnimation, FDying, FActivated>();
	SpeedLimitOverrideFilter = FFilter::Make<FCollider, FLocated, FSphereObstacle>();
	AgentPatrolFilter = FFilter::Make<FAgent, FCollider, FLocated, FPatrol, FTrace, FMove, FActivated, FRendering>().Exclude<FAppearing, FSleeping, FAttacking, FDying>();
	AgentMoveFilter = FFilter::Make<FAgent, FRendering, FAnimation, FMove, FMoving, FDirected, FLocated, FAttack, FTrace, FNavigation, FAvoidance, FCollider, FDefence, FPatrol, FActivated>();
	IdleToMoveAnimFilter = FFilter::Make<FAgent, FAnimation, FMove, FMoving, FDeath, FActivated>().Exclude<FAppearing, FDying>();
	AgentStateMachineFilter = FFilter::Make<FAgent, FAnimation, FRendering, FAppear, FAttack, FDeath, FMoving, FActivated>();
	RenderBatchFilter = FFilter::Make<FRenderBatchData>();
	AgentRenderFilter = FFilter::Make<FAgent, FRendering, FDirected, FScaled, FLocated, FAnimation, FHealth, FHealthBar, FCollider, FActivated>();
	TextRenderFilter = FFilter::Make<FAgent, FRendering, FPoppingText, FActivated>();
	SpawnActorsFilter = FFilter::Make<FActorSpawnConfig>();
	SpawnFxFilter = FFilter::Make<FFxConfig>();
	PlaySoundFilter = FFilter::Make<FSoundConfig>();
}