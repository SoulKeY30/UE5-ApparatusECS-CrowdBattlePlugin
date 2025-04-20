/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#include "BattleFrameBattleControl.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/ThreadManager.h"

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
	CurrentWorld = GetWorld();
	Mechanism = UMachine::ObtainMechanism(CurrentWorld);
	if (ANeighborGridActor::GetInstance()) { NeighborGrid = ANeighborGridActor::GetInstance()->GetComponent(); }
}

void ABattleFrameBattleControl::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("BattleControlTick");

	Super::Tick(DeltaTime);

	if (bIsGameOver || !CurrentWorld || !Mechanism || !NeighborGrid) return;

	//----------------------数据统计-------------------------

	// 统计Agent数量
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Agent Count");

		FFilter Filter = FFilter::Make<FAgent>();
		auto Chain = Mechanism->Enchain(Filter);
		AgentCount = Chain->IterableNum();
		Chain->Reset(true);
	}
	#pragma endregion

	// 统计游戏时长
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Agent Age");

		FFilter Filter = FFilter::Make<FStatistics>();
		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently([&](FStatistics& Stats)
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

		FFilter Filter = FFilter::Make<FAgent, FRendering, FLocated, FDirected, FAppear, FAppearing, FActivated>();
		auto Chain = Mechanism->EnchainSolid(Filter);
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
					}
				}
				else
				{
					Subject.RemoveTraitDeferred<FAppearing>();
				}

				Appearing.time += DeltaTime;

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 出生动画
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentAppearAnim");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FAnimation, FAppear, FAppearAnim, FActivated>();
		auto Chain = Mechanism->EnchainSolid(Filter);
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
					Animation.SubjectState = ESubjectState::Appearing;
				}
				else if (AppearAnim.animTime >= Appear.Duration)
				{
					Subject.RemoveTraitDeferred<FAppearAnim>();
				}

				AppearAnim.animTime += DeltaTime;

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 出生淡入
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentAppearDissolve");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FAppearDissolve, FAnimation, FCurves, FActivated>();
		auto Chain = Mechanism->EnchainSolid(Filter);
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

				AppearDissolve.dissolveTime += DeltaTime;

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	//----------------------攻击逻辑-------------------------

	// 索敌
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
		FFilter Filter = FFilter::Make<FAgent, FTrace, FLocated, FDirected, FCollider, FSleep, FPatrol, FActivated>();
		Filter.Exclude<FAppearing, FAttacking, FDying>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, 200, ThreadsCount, BatchSize);

		TArray<FValidSubjects> ValidSubjectsArray;
		ValidSubjectsArray.SetNum(ThreadsCount);

		// A workaround. Apparatus does not expose the array of iterables, so we have to gather manually
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

			const float FinalRange = bIsSleeping ? Sleep.WakeRadius : Trace.Radius;
			const float FinalAngle = bIsSleeping ? Sleep.WakeAngle : Trace.Angle;

			switch (Trace.Mode)
			{
				case ETraceMode::TargetIsPlayer_0:
				{
					Trace.TraceResult = FSubjectHandle{};

					if (bPlayerIsValid)
					{
						// 计算目标半径和实际距离
						float OtherRadius = Trace.TraceResult.HasTrait<FCollider>() ? Trace.TraceResult.GetTrait<FCollider>().Radius : 0;
						float Distance = FMath::Clamp(FVector::Dist(Located.Location, PlayerLocation) - Collider.Radius - OtherRadius, 0, FLT_MAX);

						// 距离检查
						if (Distance <= FinalRange)
						{
							// 角度检查
							const FVector ToPlayerDir = (PlayerLocation - Located.Location).GetSafeNormal();
							const float DotValue = FVector::DotProduct(Directed.Direction, ToPlayerDir);
							const float AngleDiff = FMath::RadiansToDegrees(FMath::Acos(DotValue));

							if (AngleDiff <= FinalAngle * 0.5f)
							{
								if (Trace.bCheckVisibility)
								{
									if (IsValid(Trace.NeighborGrid) && Trace.NeighborGrid->CheckVisibility(Located.Location, PlayerLocation, Collider.Radius))
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
					Trace.TraceResult = FSubjectHandle{};

					if (LIKELY(IsValid(Trace.NeighborGrid)))
					{
						FFilter TargetFilter;
						FSubjectHandle Result;

						TargetFilter.Include(Trace.IncludeTraits);
						TargetFilter.Exclude(Trace.ExcludeTraits);

						// 使用扇形检测替换球体检测
						const FVector TraceDirection = Directed.Direction.GetSafeNormal2D();
						const float TraceHeight = Collider.Radius * 2.0f; // 根据实际需求调整高度

						// ignore self
						TArray<FSubjectHandle> IgnoreList;
						IgnoreList.Add(FSubjectHandle(Subject));

						Trace.NeighborGrid->SectorExpandForSubject(
							Located.Location,   // 检测原点
							FinalRange,         // 检测半径
							TraceHeight,        // 检测高度
							TraceDirection,     // 扇形方向
							FinalAngle,         // 扇形角度
							Trace.bCheckVisibility,
							IgnoreList,
							TargetFilter,       // 过滤条件
							Result              // 输出结果
						);

						// 直接使用结果（扇形检测已包含角度验证）
						if (Result.IsValid())
						{
							Trace.TraceResult = Result;
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

		}, EParallelForFlags::BackgroundPriority | EParallelForFlags::Unbalanced);

		Mechanism->ApplyDeferreds();
	}
	#pragma endregion

	// 攻击触发 
	#pragma region 
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentAttackMain");

		FFilter Filter = FFilter::Make<FAgent, FAttack, FRendering, FLocated, FDirected, FTrace, FCollider, FActivated>();
		Filter.Exclude<FAppearing, FSleeping, FAttacking, FDying>();

		auto Chain = Mechanism->EnchainSolid(Filter);
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

	// 攻击过程
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentAttacking");

		FFilter Filter = FFilter::Make<FAgent, FAttack, FRendering, FLocated, FAnimation, FAttacking, FMove, FMoving, FDirected, FTrace, FDebuff, FDamage, FActivated>();
		Filter.Exclude<FAppearing, FSleeping, FDying>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FLocated& Located,
				FRendering& Rendering,
				FAnimation& Animation,
				FAttack& Attack,
				FAttacking& Attacking,
				FMove& Move,
				FMoving& Moving,
				FDirected& Directed,
				FTrace& Trace,
				FDebuff& Debuff,
				FDamage& Damage)
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

						// Melee ATK
						if (Attack.TimeOfHitAction == EAttackMode::ApplyDMG)
						{
							if (Trace.TraceResult.IsValid() && !Trace.TraceResult.HasTrait<FDying>())
							{
								if (Distance <= Attack.Range && Angle <= Attack.AngleTolerance)
								{
									FDmgSphere DmgSphere = { Damage.Damage,Damage.KineticDmg,Damage.FireDmg,Damage.IceDmg,Damage.PercentDmg,Damage.CritProbability,Damage.CritMult };
									ApplyDamageToSubjects(TArray<FSubjectHandle>{Trace.TraceResult}, TArray<FSubjectHandle>{}, FSubjectHandle{ Subject }, Located.Location, DmgSphere, Debuff);
								}
							}
						}

						// Suicide ATK
						if (Attack.TimeOfHitAction == EAttackMode::SuicideATK)
						{
							if (Trace.TraceResult.IsValid() && !Trace.TraceResult.HasTrait<FDying>())
							{
								if (Distance <= Attack.Range)
								{
									FDmgSphere DmgSphere = { Damage.Damage,Damage.KineticDmg,Damage.FireDmg,Damage.IceDmg,Damage.PercentDmg,Damage.CritProbability,Damage.CritMult };
									ApplyDamageToSubjects(TArray<FSubjectHandle>{Trace.TraceResult}, TArray<FSubjectHandle>{}, FSubjectHandle{ Subject }, Located.Location, DmgSphere, Debuff);
								}
							}
							Subject.DespawnDeferred();
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

				Attacking.Time += DeltaTime;

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	//----------------------受击逻辑-------------------------

	// 受击发光
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentHitGlow");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FHitGlow, FAnimation, FCurves, FActivated>();
		Filter.Exclude<FAppearing, FBeingHit>();

		auto Chain = Mechanism->EnchainSolid(Filter);
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
					HitGlow.GlowTime += DeltaTime;
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

	// 怪物受击形变
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentJiggle");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FJiggle, FScaled, FHit, FCurves, FActivated>();
		Filter.Exclude<FAppearing, FBeingHit>();

		auto Chain = Mechanism->EnchainSolid(Filter);
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
					Jiggle.JiggleTime += DeltaTime;
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

	// 灼烧持续掉血
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentBurning");

		auto Filter = FFilter::Make<FTemporalDamaging>();
		auto Chain = Mechanism->EnchainSolid(Filter);
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
								TargetBurning.RemainingTemporalDamager --;
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

				Temporal.TemporalDamageTimeout -= DeltaTime;

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

	// 结算伤害
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("DecideAgentDamage");

		FFilter Filter = FFilter::Make<FHealth, FLocated>();// it processes hero and prop type too
		Filter.Exclude<FAppearing, FDying>();

		auto Chain = Mechanism->EnchainSolid(Filter);
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

	// 更新血条
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentHealthBar");

		static const auto Filter = FFilter::Make<FAgent, FRendering, FHealth, FHealthBar, FActivated>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

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

		FFilter Filter = FFilter::Make<FAgent, FRendering, FDeath,FLocated, FDying, FDirected, FTrace, FMove, FMoving, FActivated>();
		Filter.Exclude<FAppearing>();

		auto Chain = Mechanism->EnchainSolid(Filter);
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

				else if (Dying.Time > Dying.Duration)
				{
					// 移除	
					Subject.DespawnDeferred();
				}

				Dying.Time += DeltaTime;

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 死亡消融
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentDeathDissolve");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FDeathDissolve, FAnimation, FDying, FDeath, FCurves, FActivated>();
		Filter.Exclude<FAppearing>();

		auto Chain = Mechanism->EnchainSolid(Filter);
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
				DeathDissolve.dissolveTime += DeltaTime;

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 死亡动画
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentDeathAnim");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FDeathAnim, FAnimation, FDying, FActivated>();
		Filter.Exclude<FAppearing>();

		auto Chain = Mechanism->EnchainSolid(Filter);
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

				DeathAnim.animTime += DeltaTime;

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	//-----------------------移动逻辑------------------------

	// 冰冻减速
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentFrozen");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FAnimation, FFreezing, FActivated>();
		Filter.Exclude<FAppearing, FDying>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

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
	}
	#pragma endregion

	// 速度覆盖
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("SpeedLimitOverride");

		FFilter Filter = FFilter::Make<FCollider, FLocated, FSphereObstacle>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FCollider Collider,
				FLocated Located,
				FSphereObstacle& SphereObstacle)
			{
				TArray<FTraceResult> Results; // 改为FTraceResult数组

				if (UNLIKELY(!IsValid(SphereObstacle.NeighborGrid))) return;

				if (!SphereObstacle.bOverrideSpeedLimit) return;

				// 调用修改后的SphereTraceForSubjects函数
				SphereObstacle.NeighborGrid->SphereTraceForSubjects(Located.Location, Collider.Radius, TArray<FSubjectHandle>(), FFilter::Make<FAgent, FLocated, FCollider, FMoving>(), Results);

				// 处理结果
				if (!Results.IsEmpty())
				{
					// 清空并重新填充OverridingAgents
					SphereObstacle.OverridingAgents.Empty();
					for (const FTraceResult& Result : Results)
					{
						if (Result.Subject.IsValid())
						{
							SphereObstacle.OverridingAgents.Add(Result.Subject);
						}
					}
				}

				if (SphereObstacle.OverridingAgents.IsEmpty()) return;

				TSet<FSubjectHandle> Agents = SphereObstacle.OverridingAgents;

				for (const auto& Agent : Agents)
				{
					if (UNLIKELY(!Agent.IsValid())) continue;

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

	// 加载流场
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

	// 巡逻
	#pragma region 
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Patrol");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FLocated, FDirected, FTrace, FPatrol, FActivated>();
		Filter.Exclude<FAppearing, FSleeping, FAttacking, FDying>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		// Lambda for finding a new goal location. To Do : Add pathfinding and obstacle tracing here
		auto FindNewGoalLocation = [&](FVector Origin, float MinRange, float MaxRange, FTrace Trace, FLocated Located)
		{
			TRACE_CPUPROFILER_EVENT_SCOPE_STR("FindNewGoalLocation");

			const int32 MaxAttempts = 1; // 最大尝试次数
			FVector NewGoal = Located.Location;

			// 获取NeighborGridComponent引用
			UNeighborGridComponent* NeighborGrid = Trace.NeighborGrid;

			if (!IsValid(NeighborGrid))
			{
				// 如果没有有效的NeighborGrid，回退到简单环形随机位置
				float Angle = FMath::FRandRange(0.f, 2.f * PI);
				float Distance = FMath::FRandRange(MinRange, MaxRange); // 使用环形范围
				return Origin + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.f);
			}

			for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
			{
				// 随机生成环形区域内的目标位置
				float Angle = FMath::FRandRange(0.f, 2.f * PI);
				float Distance = FMath::FRandRange(MinRange, MaxRange); // 确保在环形范围内
				NewGoal = Origin + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.f);

				// 使用NeighborGridComponent检查可见性
				if (NeighborGrid->CheckVisibility(Located.Location, NewGoal, Trace.Radius))
				{
					return NewGoal;
				}
			}

			// 如果多次尝试都找不到可见点，返回最后一次尝试的位置
			return NewGoal;
		};

		// Lambda for resetting timer
		auto ResetTimer = [](const FPatrol& Patrol, FPatrolling& Patrolling) 
		{
			Patrolling.MoveTimeLeft = Patrol.MaxDuration;
			Patrolling.WaitTimeLeft = Patrol.CoolDown;
		};

		// Lambda for resetting origin
		auto ResetOrigin = [](FPatrol& Patrol, const FLocated& Located) 
		{
			if (Patrol.OriginMode == EPatrolOriginMode::Previous) 
			{
				Patrol.Origin = Located.Location; // use current loc
			}
			else 
			{
				Patrol.Origin = Located.InitialLocation; // use init loc
			}
		};

		Chain->OperateConcurrently(
			[&, DeltaTime = DeltaTime](FSolidSubjectHandle Subject,
				FLocated& Located,
				FTrace& Trace,
				FPatrol& Patrol)
			{
				if (!Patrol.bEnable) return;

				// 如果有追踪目标，使用目标位置
				if (Trace.TraceResult.IsValid() && Trace.TraceResult.HasTrait<FLocated>())
				{
					Patrol.Goal = Trace.TraceResult.GetTrait<FLocated>().Location;
				}

				// 巡逻逻辑
				if (Subject.HasTrait<FPatrolling>())
				{
					auto& Patrolling = Subject.GetTraitRef<FPatrolling, EParadigm::Unsafe>();
					const float ArrivalThreshold = 10.f; // 到达阈值

					// 检查是否到达目标点
					const bool bHasArrived = FVector::Dist2D(Located.Location, Patrol.Goal) < ArrivalThreshold;

					if (bHasArrived)
					{
						// 到达目标点后的等待逻辑
						if (Patrolling.WaitTimeLeft <= 0.f)
						{
							ResetTimer(Patrol, Patrolling);
							ResetOrigin(Patrol, Located);
							Patrol.Goal = FindNewGoalLocation(Patrol.Origin,Patrol.MinRadius,Patrol.MaxRadius,Trace,Located);
						}
						else
						{
							Patrolling.WaitTimeLeft -= DeltaTime;
						}
					}
					else
					{
						// 移动超时逻辑
						if (Patrolling.MoveTimeLeft <= 0.f)
						{
							ResetTimer(Patrol, Patrolling);
							ResetOrigin(Patrol, Located);
							Patrol.Goal = FindNewGoalLocation(Patrol.Origin, Patrol.MinRadius, Patrol.MaxRadius, Trace, Located);
						}
						else
						{
							Patrolling.MoveTimeLeft -= DeltaTime;
						}
					}
				}

				// 重新进入巡逻状态的逻辑
				else if (!Trace.TraceResult.IsValid() && Patrol.OnLostTarget == EPatrolRecoverMode::Patrol)
				{
					TRACE_CPUPROFILER_EVENT_SCOPE_STR("Re-patrol");

					FPatrolling NewPatrolling;
					ResetTimer(Patrol, NewPatrolling);
					ResetOrigin(Patrol, Located);

					Subject.SetTraitDeferred(NewPatrolling);
				}

			},ThreadsCount,BatchSize);
	}
	#pragma endregion

	// 移动
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentMovement");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FAnimation, FMove, FMoving, FDirected, FLocated, FAttack, FTrace, FNavigation, FAvoidance, FCollider, FDefence, FPatrol, FActivated>();
		Filter.Exclude<FAppearing>();

		auto Chain = Mechanism->EnchainSolid(Filter);
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
				if (!Move.bEnable) return;

				// 死亡区域检测
				if (Located.Location.Z < Move.KillZ)
				{
					Subject.DespawnDeferred();
					return;
				}

				const bool bIsSleeping = Subject.HasTrait<FSleeping>();
				const bool bIsPatrolling = Subject.HasTrait<FPatrolling>();
				const bool bIsAttacking = Subject.HasTrait<FAttacking>();
				const bool bIsFreezing = Subject.HasTrait<FFreezing>();
				const bool bIsDying = Subject.HasTrait<FDying>();

				const bool bIsValidFF = IsValid(Navigation.FlowField);
				const bool bIsValidTraceResult = Trace.TraceResult.IsValid();
				const bool bIsTraceResultHasLocated = bIsValidTraceResult ? Trace.TraceResult.HasTrait<FLocated>() : false;
				const bool bIsTraceResultHasBindFlowField = bIsValidTraceResult ? Trace.TraceResult.HasTrait<FBindFlowField>() : false;

				// 初始化
				FVector& AgentLocation = Located.Location;
				FVector GoalLocation = AgentLocation;
				FVector DesiredMoveDirection = Directed.Direction;
				Moving.SpeedMult = 1;

				bool bInside_BaseFF = false;
				FCellStruct Cell_BaseFF = FCellStruct{};
				bool bInside_TargetFF = false;
				FCellStruct Cell_TargetFF = FCellStruct{};


				//----------------------------- 寻路 ----------------------------//

				// Lambda to handle direct approach logic WIP add individual navigation in here
				auto TryApproachDirectly = [&]()
				{
					if (bIsTraceResultHasLocated)
					{
						GoalLocation = Trace.TraceResult.GetTraitRef<FLocated, EParadigm::Unsafe>().Location;
						DesiredMoveDirection = (GoalLocation - AgentLocation).GetSafeNormal2D();
					}
					else
					{
						Moving.SpeedMult = 0;
					}
				};

				// 默认流场, 必须获取因为之后要用到地面高度
				if (bIsValidFF)
				{
					Navigation.FlowField->GetCellAtLocation(AgentLocation, bInside_BaseFF, Cell_BaseFF);

					if (!bIsPatrolling)
					{
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
				}

				if (bIsPatrolling)// if patrolling
				{
					GoalLocation = Patrol.Goal;
					DesiredMoveDirection = (GoalLocation - AgentLocation).GetSafeNormal2D();
				}

				if (bIsValidTraceResult) // 有目标，目标有指向目标自己的流场
				{
					if (bIsTraceResultHasBindFlowField)
					{
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
					}
					else
					{
						TryApproachDirectly();
					}
				}

				//--------------------------- 计算水平速度 ----------------------------//

				// 助推力
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
				if (!bIsSleeping && !Moving.bFalling && !bIsDying)
				{
					float SlowFactor = 1;

					if (bIsFreezing)
					{
						// 减速转向
						const auto& Freezing = Subject.GetTraitRef<FFreezing, EParadigm::Unsafe>();
						SlowFactor = 1.0f - Freezing.FreezeStr;
					}

					float DeltaV = FMath::Abs(Moving.DesiredVelocity.Size2D() - Moving.CurrentVelocity.Size2D());

					if (AngleDegrees < 90.f && DeltaV > 100.f)// 一致，朝向实际移动的方向
					{
						if (!(bIsAttacking && Subject.GetTrait<FAttacking>().State != EAttackState::Cooling))
						{
							Directed.DesiredRot = Moving.CurrentVelocity.GetSafeNormal2D().ToOrientationRotator();//the current velocity direction
						}

						const FRotator CurrentRot = Directed.Direction.ToOrientationRotator();
						const FRotator InterpolatedRot = FMath::RInterpTo(CurrentRot, Directed.DesiredRot, DeltaTime, Move.TurnSpeed * SlowFactor);
						Directed.Direction = InterpolatedRot.Vector();
					}
					else// 不一致，朝向想要移动的方向
					{
						if (!(bIsAttacking && Subject.GetTrait<FAttacking>().State != EAttackState::Cooling))
						{
							Directed.DesiredRot = DesiredMoveDirection.GetSafeNormal2D().ToOrientationRotator();
						}

						const FRotator CurrentRot = Directed.Direction.ToOrientationRotator();
						const FRotator InterpolatedRot = FMath::RInterpTo(CurrentRot, Directed.DesiredRot, DeltaTime, Move.TurnSpeed * SlowFactor);
						Directed.Direction = InterpolatedRot.Vector();
						DesiredMoveDirection = Directed.Direction;
					}
				}

				// 减速
				if (bIsSleeping || bIsAttacking || bIsDying)
				{
					Moving.SpeedMult = 0.0f; // 停止移动
				}
				else if (bIsFreezing)
				{
					const auto& Freezing = Subject.GetTraitRef<FFreezing, EParadigm::Unsafe>();

					if (Freezing.FreezeTimeout > 0.0f)
					{
						Moving.SpeedMult *= (1.0f - Freezing.FreezeStr); //冰冻时减速
					}
				}

				// 速度-距离曲线
				float OtherRadius = Trace.TraceResult.HasTrait<FCollider>() ? Trace.TraceResult.GetTrait<FCollider>().Radius : 0;
				const float DistanceToGoal = FMath::Clamp(FVector::Dist2D(AgentLocation, GoalLocation) - OtherRadius, 0, FLT_MAX);
				const TRange<float> MoveInputRange(Move.MoveSpeedRangeMap.Y, Move.MoveSpeedRangeMap.W);
				const TRange<float> MoveOutputRange(Move.MoveSpeedRangeMap.X, Move.MoveSpeedRangeMap.Z);
				Moving.SpeedMult *= FMath::GetMappedRangeValueClamped(MoveInputRange, MoveOutputRange, DistanceToGoal);

				// 想要达到的理想速度
				FVector DesiredVelocity = Moving.SpeedMult * Move.MoveSpeed * DesiredMoveDirection;
				Moving.DesiredVelocity = FVector(DesiredVelocity.X, DesiredVelocity.Y, 0);

				//---------------------------计算垂直速度-------------------------//

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
				}

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	// 碰撞与避障
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("Avoidance");// agent only do avoidance in xy, not z
		NeighborGrid->Evaluate();
	}
	#pragma endregion

	// 移动动画
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("IdleToMoveAnim");

		// 初始化过滤器
		FFilter Filter = FFilter::Make<FAgent, FAnimation, FMoving, FDeath, FActivated>();
		Filter.Exclude<FAppearing>();

		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, 1, ThreadsCount, BatchSize);

		Chain->OperateConcurrently(
			[&](FSolidSubjectHandle Subject,
				FAnimation& Animation,
				FMoving& Moving,
				FDeath& Death)
			{
				const bool bIsAttacking = Subject.HasTrait<FAttacking>();
				const bool bIsDying = Subject.HasTrait<FDying>();

				float RealSpeed2D = Moving.CurrentVelocity.Size2D();

				if (LIKELY(!bIsAttacking && !bIsDying))
				{
					Animation.SubjectState = (RealSpeed2D > Animation.UseMoveAnimAboveSpeed) ? ESubjectState::Moving : ESubjectState::Idle;// switch between idle and move based on speed(ignore small value)
				}
				else if (bIsDying && RealSpeed2D < 100.f && Death.bDisableCollision && !Subject.HasTrait<FCorpse>())
				{
					Subject.SetTraitDeferred(FCorpse{});
				}

			}, ThreadsCount, BatchSize);
	}
	#pragma endregion

	//------------------------更新渲染------------------------

	// 动画状态机
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentStateMachine");
		static const auto Filter = FFilter::Make<FAgent, FAnimation, FRendering, FAppear, FAttack, FDeath, FMoving, FActivated>();

		auto Chain = Mechanism->EnchainSolid(Filter);
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

	// 合批渲染数据
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AgentRender");

		FFilter Filter = FFilter::Make<FAgent, FRendering, FDirected, FScaled, FLocated, FAnimation, FHealth, FHealthBar, FCollider, FActivated>();

		auto Chain = Mechanism->EnchainSolid(Filter);
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

		FFilter Filter = FFilter::Make<FAgent, FRendering, FPoppingText, FActivated>();

		auto Chain = Mechanism->EnchainSolid(Filter);
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

	// Write Pooling Info
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("WritePoolingInfo");

		FFilter Filter = FFilter::Make<FRenderBatchData>().Exclude<FDying>();

		auto Chain = Mechanism->EnchainSolid(Filter);
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

	//---------------------Other Operates---------------------

	// Spawn Actors
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("SpawnActors");

		FFilter Filter = FFilter::Make<FActorSpawnConfig>();

		Mechanism->Operate<FUnsafeChain>(Filter,
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
					Config.Delay -= DeltaTime;
				}
			});
	}
	#pragma endregion

	// Spawn Fx
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("SpawnFx");

		FFilter Filter = FFilter::Make<FFxConfig>();

		Mechanism->Operate<FUnsafeChain>(Filter,
			[&](FSubjectHandle Subject,
				FFxConfig& Config)
			{
				if (Config.Delay <= 0)
				{
					// 验证合批情况下的SubType
					if (Config.SubType != ESubType::None)
					{
						FLocated FxLocated = { Config.Transform.GetLocation() };
						FDirected FxDirected = { Config.Transform.GetRotation().GetForwardVector() };
						FScaled FxScaled = { Config.Transform.GetScale3D() };

						FSubjectRecord FxRecord;
						FxRecord.SetTrait(FSpawningFx{});
						FxRecord.SetTrait(FxLocated);
						FxRecord.SetTrait(FxDirected);
						FxRecord.SetTrait(FxScaled);

						UBattleFrameFunctionLibraryRT::SetSubTypeTraitByEnum(Config.SubType, FxRecord);

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
					Config.Delay -= DeltaTime;
				}
			});
	}
	#pragma endregion

	// Play Sound
	#pragma region
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("PlaySound");

		FFilter Filter = FFilter::Make<FSoundConfig>();

		Mechanism->Operate<FUnsafeChain>(Filter,
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
					Config.Delay -= DeltaTime;
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

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FDmgResult ABattleFrameBattleControl::ApplyDamageToSubjects(TArray<FSubjectHandle> Subjects, TArray<FSubjectHandle> IgnoreSubjects, FSubjectHandle DmgInstigator, FVector HitFromLocation, FDmgSphere DmgSphere, FDebuff Debuff)
{
	// Record for deferred spawning of TemporalDamager
	FTemporalDamaging TemporalDamaging;

	// 使用TSet存储唯一敌人句柄
	TSet<FSubjectHandle> UniqueHandles;

	// 将IgnoreSubjects转换为TSet以提高查找效率
	const TSet<FSubjectHandle> IgnoreSet(IgnoreSubjects);

	FDmgResult Result;

	for (const auto& Overlapper : Subjects)
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
		const bool bHasDefence = Overlapper.HasTrait<FDefence>();
		const bool bHasTextPopUp = Overlapper.HasTrait<FTextPopUp>();
		const bool bHasCollider = Overlapper.HasTrait<FCollider>();
		const bool bHasMove = Overlapper.HasTrait<FMove>();
		const bool bHasMoving = Overlapper.HasTrait<FMoving>();
		const bool bHasFreezing = Overlapper.HasTrait<FFreezing>();
		const bool bHasBurning = Overlapper.HasTrait<FBurning>();
		const bool bHasAnimation = Overlapper.HasTrait<FAnimation>();
		const bool bHasSleeping = Overlapper.HasTrait<FSleeping>();
		const bool bHasSleep = Overlapper.HasTrait<FSleep>();
		const bool bHasHit = Overlapper.HasTrait<FHit>();
		const bool bHasHitGlow = Overlapper.HasTrait<FHitGlow>();
		const bool bHasJiggle = Overlapper.HasTrait<FJiggle>();

		FVector Location = bHasLocated ? Overlapper.GetTrait<FLocated>().Location : FVector::ZeroVector;
		FVector Direction = bHasDirected ? Overlapper.GetTrait<FDirected>().Direction : FVector::ZeroVector;

		//-------------伤害和抗性------------

		float NormalDmgMult = 1;
		float KineticDmgMult = 1;
		//float KineticDebuffMult = 1;
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
				//KineticDebuffMult = 1 - Defence.KineticDebuffImmune;
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
	}

	return Result;
}
