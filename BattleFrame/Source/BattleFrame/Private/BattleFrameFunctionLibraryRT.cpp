// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleFrameFunctionLibraryRT.h"
#include "Traits/Located.h"
#include "Traits/AvoGroup.h"
#include "Traits/Team.h"
#include "SubjectHandle.h"
#include "SubjectRecord.h"
#include "EngineUtils.h"
#include "BattleFrameBattleControl.h"
#include "NeighborGridActor.h"
#include "Async/Async.h"

//-------------------------------Sync Traces-------------------------------

void UBattleFrameFunctionLibraryRT::SphereTraceForSubjects
(
	ANeighborGridActor* NeighborGridActor,
	int32 KeepCount,
	FVector Origin,
	float Radius,
	bool bCheckVisibility,
	FVector CheckOrigin,
	float CheckRadius,
	ESortMode SortMode,
	const FVector SortOrigin,
	UPARAM(ref) const TArray<FSubjectHandle>& IgnoreSubjects,
	UPARAM(ref) const FFilter& Filter,
	bool& Hit,
	TArray<FTraceResult>& TraceResults
)
{
	TraceResults.Reset();

	if (!IsValid(NeighborGridActor))
	{
		if (UWorld* World = GEngine->GetCurrentPlayWorld())
		{
			for (TActorIterator<ANeighborGridActor> It(World); It; ++It)
			{
				NeighborGridActor = *It;
				break;
			}
		}
	}

	if (!IsValid(NeighborGridActor)) return;

	UNeighborGridComponent* NeighborGrid = NeighborGridActor->GetComponentByClass<UNeighborGridComponent>();

	NeighborGrid->SphereTraceForSubjects(KeepCount, Origin, Radius, bCheckVisibility, CheckOrigin, CheckRadius, SortMode, SortOrigin, IgnoreSubjects, Filter, Hit, TraceResults);
}

void UBattleFrameFunctionLibraryRT::SphereSweepForSubjects
(
	ANeighborGridActor* NeighborGridActor,
	int32 KeepCount,
	FVector Start,
	FVector End,
	float Radius,
	bool bCheckVisibility,
	const FVector CheckOrigin,
	float CheckRadius,
	ESortMode SortMode,
	const FVector SortOrigin,
	UPARAM(ref) const TArray<FSubjectHandle>& IgnoreSubjects,
	UPARAM(ref) const FFilter& Filter,
	bool& Hit,
	TArray<FTraceResult>& TraceResults
)
{
	TraceResults.Reset();

	if (!IsValid(NeighborGridActor))
	{
		if (UWorld* World = GEngine->GetCurrentPlayWorld())
		{
			for (TActorIterator<ANeighborGridActor> It(World); It; ++It)
			{
				NeighborGridActor = *It;
				break;
			}
		}
	}

	if (!IsValid(NeighborGridActor)) return;

	UNeighborGridComponent* NeighborGrid = NeighborGridActor->GetComponentByClass<UNeighborGridComponent>();

	NeighborGrid->SphereSweepForSubjects(KeepCount, Start, End, Radius, bCheckVisibility, CheckOrigin, CheckRadius, SortMode, SortOrigin, IgnoreSubjects, Filter, Hit, TraceResults);
}

void UBattleFrameFunctionLibraryRT::SectorTraceForSubjects
(
	ANeighborGridActor* NeighborGridActor,
	int32 KeepCount,
	FVector Origin,
	float Radius,
	float Height,
	FVector Direction,
	float Angle,
	bool bCheckVisibility,
	const FVector CheckOrigin,
	float CheckRadius,
	ESortMode SortMode,
	const FVector SortOrigin,
	UPARAM(ref) const TArray<FSubjectHandle>& IgnoreSubjects,
	UPARAM(ref) const FFilter& Filter,
	bool& Hit,
	TArray<FTraceResult>& TraceResults
)
{
	if (!IsValid(NeighborGridActor))
	{
		if (UWorld* World = GEngine->GetCurrentPlayWorld())
		{
			for (TActorIterator<ANeighborGridActor> It(World); It; ++It)
			{
				NeighborGridActor = *It;
				break;
			}
		}
	}

	if (!IsValid(NeighborGridActor)) return;

	UNeighborGridComponent* NeighborGrid = NeighborGridActor->GetComponentByClass<UNeighborGridComponent>();

	NeighborGrid->SectorTraceForSubjects(KeepCount, Origin, Radius, Height, Direction, Angle, bCheckVisibility, CheckOrigin, CheckRadius, SortMode, SortOrigin, IgnoreSubjects, Filter, Hit, TraceResults);
}

void UBattleFrameFunctionLibraryRT::SphereSweepForObstacle
(
	ANeighborGridActor* NeighborGridActor,
	FVector Start,
	FVector End,
	float Radius,
	bool& Hit,
	FTraceResult& TraceResult
)
{
	if (!IsValid(NeighborGridActor))
	{
		if (UWorld* World = GEngine->GetCurrentPlayWorld())
		{
			for (TActorIterator<ANeighborGridActor> It(World); It; ++It)
			{
				NeighborGridActor = *It;
				break;
			}
		}
	}

	if (!IsValid(NeighborGridActor)) return;

	UNeighborGridComponent* NeighborGrid = NeighborGridActor->GetComponentByClass<UNeighborGridComponent>();

	NeighborGrid->SphereSweepForObstacle(Start, End, Radius, Hit, TraceResult);
}

void UBattleFrameFunctionLibraryRT::ApplyDamageToSubjects
(
	ABattleFrameBattleControl* BattleControl,
	UPARAM(ref) const TArray<FSubjectHandle>& Subjects,
	UPARAM(ref) const TArray<FSubjectHandle>& IgnoreSubjects,
	FSubjectHandle DmgInstigator,
	FVector HitFromLocation,
	FDmgSphere DmgSphere,
	FDebuff Debuff,
	TArray<FDmgResult>& DamageResults
)
{
	DamageResults.Reset();

	// 如果 BattleControl 无效，尝试从 World 查找
	if (!IsValid(BattleControl))
	{
		UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;

		if (World)
		{
			for (TActorIterator<ABattleFrameBattleControl> It(World); It; ++It)
			{
				BattleControl = *It;
				break; // 只取第一个
			}
		}
	}

	// 仍然无效则返回空结果
	if (!IsValid(BattleControl)) return;

	// 直接填充 DamageResults
	BattleControl->ApplyDamageToSubjects(Subjects, IgnoreSubjects, DmgInstigator, HitFromLocation, DmgSphere, Debuff, DamageResults);
}

void UBattleFrameFunctionLibraryRT::SortSubjectsByDistance
(
	UPARAM(ref) TArray<FTraceResult>& TraceResults,
	const FVector& SortOrigin,
	ESortMode SortMode
)
{
	// 1. 首先移除所有无效的Subject
	TraceResults.RemoveAll([](const FTraceResult& TraceResult)
	{
		return !TraceResult.Subject.IsValid();
	});

	// 2. 检查剩余元素数量
	if (TraceResults.Num() <= 1)
	{
		return; // 不需要排序
	}

	// 3. 预计算距离（可选优化）
	for (auto& Result : TraceResults)
	{
		Result.CachedDistSq = FVector::DistSquared(SortOrigin, Result.Location);
	}

	// 4. 根据模式排序
	TraceResults.Sort([SortMode](const FTraceResult& A, const FTraceResult& B)
	{
		if (SortMode == ESortMode::NearToFar)
		{
			return A.CachedDistSq < B.CachedDistSq;  // 从近到远
		}
		else
		{
			return A.CachedDistSq > B.CachedDistSq;  // 从远到近
		}
	});
}


//-------------------------------Async Trace-------------------------------

USphereSweepForSubjectsAsyncAction* USphereSweepForSubjectsAsyncAction::SphereSweepForSubjectsAsync
(
	const UObject* WorldContextObject,
	ANeighborGridActor* NeighborGridActor,
	const int32 KeepCount,
	const FVector Start,
	const FVector End,
	const float Radius,
	const bool bCheckVisibility,
	const FVector CheckOrigin,
	const float CheckRadius,
	const ESortMode SortMode,
	const FVector SortOrigin,
	const TArray<FSubjectHandle>& IgnoreSubjects,
	const FFilter Filter
)
{
	USphereSweepForSubjectsAsyncAction* AsyncAction = NewObject<USphereSweepForSubjectsAsyncAction>();
	AsyncAction->RegisterWithGameInstance(WorldContextObject ? WorldContextObject->GetWorld() : nullptr);

	if (!IsValid(NeighborGridActor))
	{
		if (UWorld* World = WorldContextObject->GetWorld())
		{
			for (TActorIterator<ANeighborGridActor> It(World); It; ++It)
			{
				NeighborGridActor = *It;
				break;
			}
		}
	}

	AsyncAction->NeighborGridActor = NeighborGridActor;
	AsyncAction->Start = Start;
	AsyncAction->End = End;
	AsyncAction->Radius = Radius;
	AsyncAction->bCheckVisibility = bCheckVisibility;
	AsyncAction->CheckOrigin = CheckOrigin;
	AsyncAction->CheckRadius = CheckRadius;
	AsyncAction->SortMode = SortMode;
	AsyncAction->SortOrigin = SortOrigin;
	AsyncAction->KeepCount = KeepCount;
	AsyncAction->Filter = Filter;
	AsyncAction->IgnoreSubjects = IgnoreSubjects;

	return AsyncAction;
}

void USphereSweepForSubjectsAsyncAction::Activate()
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("SphereSweepForSubjectsAsync");

	if (!IsValid(NeighborGridActor.Get()))
	{
		Hit = false;
		Completed.Broadcast(Hit, Results);
		SetReadyToDestroy();
	}
	else
	{
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
			{
				//TRACE_CPUPROFILER_EVENT_SCOPE_STR("SphereSweepForSubjectsAsync");

				TWeakObjectPtr<UNeighborGridComponent> NeighborGrid = NeighborGridActor->FindComponentByClass<UNeighborGridComponent>();
				TArray<FIntVector> CellCoords = NeighborGrid->SphereSweepForCells(Start, End, Radius);

				const FVector TraceDir = (End - Start).GetSafeNormal();
				const float TraceLength = FVector::Distance(Start, End);

				// 创建忽略列表的哈希集合以便快速查找
				TSet<FSubjectHandle> IgnoreSet;

				for (const FSubjectHandle& Subject : IgnoreSubjects) IgnoreSet.Add(Subject);

				// 检查每个单元中的subject
				for (FIntVector CellCoord : CellCoords)
				{
					const FNeighborGridCell& CageCell = NeighborGrid->Cells[NeighborGrid->GetIndexAt(CellCoord)];

					if (!CageCell.SubjectFingerprint.Matches(Filter.GetFingerprint())) continue;

					for (const FAvoiding& Data : CageCell.Subjects)
					{
						const FSubjectHandle Subject = Data.SubjectHandle;

						// 检查是否在忽略列表中
						if (IgnoreSet.Contains(Subject)) continue;

						const FVector SubjectPos = Data.Location;
						float SubjectRadius = Data.Radius;

						// 距离计算
						const FVector ToSubject = SubjectPos - Start;
						const float ProjOnTrace = FVector::DotProduct(ToSubject, TraceDir);

						// 初步筛选
						const float ProjThreshold = SubjectRadius + Radius;
						if (ProjOnTrace < -ProjThreshold || ProjOnTrace > TraceLength + ProjThreshold) continue;

						// 精确距离检查
						const float ClampedProj = FMath::Clamp(ProjOnTrace, 0.0f, TraceLength);
						const FVector NearestPoint = Start + ClampedProj * TraceDir;
						const float CombinedRadSq = FMath::Square(Radius + SubjectRadius);

						if (FVector::DistSquared(NearestPoint, SubjectPos) >= CombinedRadSq) continue;

						// 可见性检查
						if (bCheckVisibility)
						{
							bool HitObstacle = false;
							FTraceResult TraceResult;
							NeighborGrid->SphereSweepForObstacle(Start, SubjectPos, CheckRadius, HitObstacle, TraceResult);

							if (HitObstacle) continue; // 路径被阻挡，跳过该目标
						}

						// 创建FTraceResult并添加到结果数组
						FTraceResult Result{ Subject ,SubjectPos ,FVector::DistSquared(SortOrigin, SubjectPos) };
						TempResults.Add(Result);
					}
				}

				// 排序逻辑（在后台线程执行）
				if (SortMode != ESortMode::None)
				{
					TempResults.Sort([this](const FTraceResult& A, const FTraceResult& B)
					{
						if (SortMode == ESortMode::NearToFar)
						{
							return A.CachedDistSq < B.CachedDistSq;
						}
						else // FarToNear
						{
							return A.CachedDistSq > B.CachedDistSq;
						}
					});
				}

				AsyncTask(ENamedThreads::GameThread, [this]()
				{
					//TRACE_CPUPROFILER_EVENT_SCOPE_STR("SphereSweepForSubjectsSync");
					Results.Reset();

					int32 ValidCount = 0;
					const bool bRequireLimit = (KeepCount > 0);

					// 按预排序顺序遍历，遇到有效项立即收集
					for (const FTraceResult& TempResult : TempResults)
					{
						if (!TempResult.Subject.Matches(Filter)) continue;// this can only run on gamethread

						Results.Add(TempResult);
						ValidCount++;

						// 达到数量限制立即终止
						if (bRequireLimit && ValidCount >= KeepCount) break;
					}

					Hit = !Results.IsEmpty();
					Completed.Broadcast(Hit, Results);
					SetReadyToDestroy();
				});
			});
		});
	}
}

void UBattleFrameFunctionLibraryRT::SetRecordSubTypeTraitByIndex(int32 Index, FSubjectRecord& SubjectRecord)
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("SetRecordSubTypeTraitByIndex");
	switch (Index)
	{
	default:
		SubjectRecord.SetTrait(FSubType0());
		break;
	case 0:
		SubjectRecord.SetTrait(FSubType0());
		break;
	case 1:
		SubjectRecord.SetTrait(FSubType1());
		break;
	case 2:
		SubjectRecord.SetTrait(FSubType2());
		break;
	case 3:
		SubjectRecord.SetTrait(FSubType3());
		break;
	case 4:
		SubjectRecord.SetTrait(FSubType4());
		break;
	case 5:
		SubjectRecord.SetTrait(FSubType5());
		break;
	case 6:
		SubjectRecord.SetTrait(FSubType6());
		break;
	case 7:
		SubjectRecord.SetTrait(FSubType7());
		break;
	case 8:
		SubjectRecord.SetTrait(FSubType8());
		break;
	case 9:
		SubjectRecord.SetTrait(FSubType9());
		break;
	case 10:
		SubjectRecord.SetTrait(FSubType10());
		break;
	case 11:
		SubjectRecord.SetTrait(FSubType11());
		break;
	case 12:
		SubjectRecord.SetTrait(FSubType12());
		break;
	case 13:
		SubjectRecord.SetTrait(FSubType13());
		break;
	case 14:
		SubjectRecord.SetTrait(FSubType14());
		break;
	case 15:
		SubjectRecord.SetTrait(FSubType15());
		break;
	case 16:
		SubjectRecord.SetTrait(FSubType16());
		break;
	case 17:
		SubjectRecord.SetTrait(FSubType17());
		break;
	case 18:
		SubjectRecord.SetTrait(FSubType18());
		break;
	case 19:
		SubjectRecord.SetTrait(FSubType19());
		break;
	case 20:
		SubjectRecord.SetTrait(FSubType20());
		break;
	case 21:
		SubjectRecord.SetTrait(FSubType21());
		break;
	case 22:
		SubjectRecord.SetTrait(FSubType22());
		break;
	case 23:
		SubjectRecord.SetTrait(FSubType23());
		break;
	case 24:
		SubjectRecord.SetTrait(FSubType24());
		break;
	case 25:
		SubjectRecord.SetTrait(FSubType25());
		break;
	case 26:
		SubjectRecord.SetTrait(FSubType26());
		break;
	case 27:
		SubjectRecord.SetTrait(FSubType27());
		break;
	case 28:
		SubjectRecord.SetTrait(FSubType28());
		break;
	case 29:
		SubjectRecord.SetTrait(FSubType29());
		break;
	case 30:
		SubjectRecord.SetTrait(FSubType30());
		break;
	case 31:
		SubjectRecord.SetTrait(FSubType31());
		break;
	case 32:
		SubjectRecord.SetTrait(FSubType32());
		break;
	case 33:
		SubjectRecord.SetTrait(FSubType33());
		break;
	case 34:
		SubjectRecord.SetTrait(FSubType34());
		break;
	case 35:
		SubjectRecord.SetTrait(FSubType35());
		break;
	case 36:
		SubjectRecord.SetTrait(FSubType36());
		break;
	case 37:
		SubjectRecord.SetTrait(FSubType37());
		break;
	case 38:
		SubjectRecord.SetTrait(FSubType38());
		break;
	case 39:
		SubjectRecord.SetTrait(FSubType39());
		break;
	case 40:
		SubjectRecord.SetTrait(FSubType40());
		break;

	case 41:
		SubjectRecord.SetTrait(FSubType41());
		break;

	case 42:
		SubjectRecord.SetTrait(FSubType42());
		break;

	case 43:
		SubjectRecord.SetTrait(FSubType43());
		break;

	case 44:
		SubjectRecord.SetTrait(FSubType44());
		break;

	case 45:
		SubjectRecord.SetTrait(FSubType45());
		break;

	case 46:
		SubjectRecord.SetTrait(FSubType46());
		break;

	case 47:
		SubjectRecord.SetTrait(FSubType47());
		break;

	case 48:
		SubjectRecord.SetTrait(FSubType48());
		break;

	case 49:
		SubjectRecord.SetTrait(FSubType49());
		break;

	case 50:
		SubjectRecord.SetTrait(FSubType50());
		break;

	case 51:
		SubjectRecord.SetTrait(FSubType51());
		break;

	case 52:
		SubjectRecord.SetTrait(FSubType52());
		break;

	case 53:
		SubjectRecord.SetTrait(FSubType53());
		break;

	case 54:
		SubjectRecord.SetTrait(FSubType54());
		break;

	case 55:
		SubjectRecord.SetTrait(FSubType55());
		break;

	case 56:
		SubjectRecord.SetTrait(FSubType56());
		break;

	case 57:
		SubjectRecord.SetTrait(FSubType57());
		break;

	case 58:
		SubjectRecord.SetTrait(FSubType58());
		break;

	case 59:
		SubjectRecord.SetTrait(FSubType59());
		break;

	case 60:
		SubjectRecord.SetTrait(FSubType60());
		break;

	case 61:
		SubjectRecord.SetTrait(FSubType61());
		break;

	case 62:
		SubjectRecord.SetTrait(FSubType62());
		break;

	case 63:
		SubjectRecord.SetTrait(FSubType63());
		break;

	case 64:
		SubjectRecord.SetTrait(FSubType64());
		break;

	case 65:
		SubjectRecord.SetTrait(FSubType65());
		break;

	case 66:
		SubjectRecord.SetTrait(FSubType66());
		break;

	case 67:
		SubjectRecord.SetTrait(FSubType67());
		break;

	case 68:
		SubjectRecord.SetTrait(FSubType68());
		break;

	case 69:
		SubjectRecord.SetTrait(FSubType69());
		break;

	case 70:
		SubjectRecord.SetTrait(FSubType70());
		break;

	case 71:
		SubjectRecord.SetTrait(FSubType71());
		break;

	case 72:
		SubjectRecord.SetTrait(FSubType72());
		break;

	case 73:
		SubjectRecord.SetTrait(FSubType73());
		break;

	case 74:
		SubjectRecord.SetTrait(FSubType74());
		break;

	case 75:
		SubjectRecord.SetTrait(FSubType75());
		break;

	case 76:
		SubjectRecord.SetTrait(FSubType76());
		break;

	case 77:
		SubjectRecord.SetTrait(FSubType77());
		break;

	case 78:
		SubjectRecord.SetTrait(FSubType78());
		break;

	case 79:
		SubjectRecord.SetTrait(FSubType79());
		break;

	case 80:
		SubjectRecord.SetTrait(FSubType80());
		break;

	case 81:
		SubjectRecord.SetTrait(FSubType81());
		break;
	}
}

void UBattleFrameFunctionLibraryRT::SetRecordSubTypeTraitByEnum(ESubType SubType, FSubjectRecord& SubjectRecord)
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("SetRecordSubTypeTraitByEnum");
	switch (SubType)
	{
	default:
		break;

	case ESubType::None:
		break;

	case ESubType::SubType0:
		SubjectRecord.SetTrait(FSubType0());
		break;
	case ESubType::SubType1:
		SubjectRecord.SetTrait(FSubType1());
		break;
	case ESubType::SubType2:
		SubjectRecord.SetTrait(FSubType2());
		break;
	case ESubType::SubType3:
		SubjectRecord.SetTrait(FSubType3());
		break;
	case ESubType::SubType4:
		SubjectRecord.SetTrait(FSubType4());
		break;
	case ESubType::SubType5:
		SubjectRecord.SetTrait(FSubType5());
		break;
	case ESubType::SubType6:
		SubjectRecord.SetTrait(FSubType6());
		break;
	case ESubType::SubType7:
		SubjectRecord.SetTrait(FSubType7());
		break;
	case ESubType::SubType8:
		SubjectRecord.SetTrait(FSubType8());
		break;
	case ESubType::SubType9:
		SubjectRecord.SetTrait(FSubType9());
		break;
	case ESubType::SubType10:
		SubjectRecord.SetTrait(FSubType10());
		break;
	case ESubType::SubType11:
		SubjectRecord.SetTrait(FSubType11());
		break;
	case ESubType::SubType12:
		SubjectRecord.SetTrait(FSubType12());
		break;
	case ESubType::SubType13:
		SubjectRecord.SetTrait(FSubType13());
		break;
	case ESubType::SubType14:
		SubjectRecord.SetTrait(FSubType14());
		break;
	case ESubType::SubType15:
		SubjectRecord.SetTrait(FSubType15());
		break;
	case ESubType::SubType16:
		SubjectRecord.SetTrait(FSubType16());
		break;
	case ESubType::SubType17:
		SubjectRecord.SetTrait(FSubType17());
		break;
	case ESubType::SubType18:
		SubjectRecord.SetTrait(FSubType18());
		break;
	case ESubType::SubType19:
		SubjectRecord.SetTrait(FSubType19());
		break;
	case ESubType::SubType20:
		SubjectRecord.SetTrait(FSubType20());
		break;
	case ESubType::SubType21:
		SubjectRecord.SetTrait(FSubType21());
		break;
	case ESubType::SubType22:
		SubjectRecord.SetTrait(FSubType22());
		break;
	case ESubType::SubType23:
		SubjectRecord.SetTrait(FSubType23());
		break;
	case ESubType::SubType24:
		SubjectRecord.SetTrait(FSubType24());
		break;
	case ESubType::SubType25:
		SubjectRecord.SetTrait(FSubType25());
		break;
	case ESubType::SubType26:
		SubjectRecord.SetTrait(FSubType26());
		break;
	case ESubType::SubType27:
		SubjectRecord.SetTrait(FSubType27());
		break;
	case ESubType::SubType28:
		SubjectRecord.SetTrait(FSubType28());
		break;
	case ESubType::SubType29:
		SubjectRecord.SetTrait(FSubType29());
		break;
	case ESubType::SubType30:
		SubjectRecord.SetTrait(FSubType30());
		break;
	case ESubType::SubType31:
		SubjectRecord.SetTrait(FSubType31());
		break;
	case ESubType::SubType32:
		SubjectRecord.SetTrait(FSubType32());
		break;
	case ESubType::SubType33:
		SubjectRecord.SetTrait(FSubType33());
		break;
	case ESubType::SubType34:
		SubjectRecord.SetTrait(FSubType34());
		break;
	case ESubType::SubType35:
		SubjectRecord.SetTrait(FSubType35());
		break;
	case ESubType::SubType36:
		SubjectRecord.SetTrait(FSubType36());
		break;
	case ESubType::SubType37:
		SubjectRecord.SetTrait(FSubType37());
		break;
	case ESubType::SubType38:
		SubjectRecord.SetTrait(FSubType38());
		break;
	case ESubType::SubType39:
		SubjectRecord.SetTrait(FSubType39());
		break;
	case ESubType::SubType40:
		SubjectRecord.SetTrait(FSubType40());
		break;

	case ESubType::SubType41:
		SubjectRecord.SetTrait(FSubType41());
		break;

	case ESubType::SubType42:
		SubjectRecord.SetTrait(FSubType42());
		break;

	case ESubType::SubType43:
		SubjectRecord.SetTrait(FSubType43());
		break;

	case ESubType::SubType44:
		SubjectRecord.SetTrait(FSubType44());
		break;

	case ESubType::SubType45:
		SubjectRecord.SetTrait(FSubType45());
		break;

	case ESubType::SubType46:
		SubjectRecord.SetTrait(FSubType46());
		break;

	case ESubType::SubType47:
		SubjectRecord.SetTrait(FSubType47());
		break;

	case ESubType::SubType48:
		SubjectRecord.SetTrait(FSubType48());
		break;

	case ESubType::SubType49:
		SubjectRecord.SetTrait(FSubType49());
		break;

	case ESubType::SubType50:
		SubjectRecord.SetTrait(FSubType50());
		break;

	case ESubType::SubType51:
		SubjectRecord.SetTrait(FSubType51());
		break;

	case ESubType::SubType52:
		SubjectRecord.SetTrait(FSubType52());
		break;

	case ESubType::SubType53:
		SubjectRecord.SetTrait(FSubType53());
		break;

	case ESubType::SubType54:
		SubjectRecord.SetTrait(FSubType54());
		break;

	case ESubType::SubType55:
		SubjectRecord.SetTrait(FSubType55());
		break;

	case ESubType::SubType56:
		SubjectRecord.SetTrait(FSubType56());
		break;

	case ESubType::SubType57:
		SubjectRecord.SetTrait(FSubType57());
		break;

	case ESubType::SubType58:
		SubjectRecord.SetTrait(FSubType58());
		break;

	case ESubType::SubType59:
		SubjectRecord.SetTrait(FSubType59());
		break;

	case ESubType::SubType60:
		SubjectRecord.SetTrait(FSubType60());
		break;

	case ESubType::SubType61:
		SubjectRecord.SetTrait(FSubType61());
		break;

	case ESubType::SubType62:
		SubjectRecord.SetTrait(FSubType62());
		break;

	case ESubType::SubType63:
		SubjectRecord.SetTrait(FSubType63());
		break;

	case ESubType::SubType64:
		SubjectRecord.SetTrait(FSubType64());
		break;

	case ESubType::SubType65:
		SubjectRecord.SetTrait(FSubType65());
		break;

	case ESubType::SubType66:
		SubjectRecord.SetTrait(FSubType66());
		break;

	case ESubType::SubType67:
		SubjectRecord.SetTrait(FSubType67());
		break;

	case ESubType::SubType68:
		SubjectRecord.SetTrait(FSubType68());
		break;

	case ESubType::SubType69:
		SubjectRecord.SetTrait(FSubType69());
		break;

	case ESubType::SubType70:
		SubjectRecord.SetTrait(FSubType70());
		break;

	case ESubType::SubType71:
		SubjectRecord.SetTrait(FSubType71());
		break;

	case ESubType::SubType72:
		SubjectRecord.SetTrait(FSubType72());
		break;

	case ESubType::SubType73:
		SubjectRecord.SetTrait(FSubType73());
		break;

	case ESubType::SubType74:
		SubjectRecord.SetTrait(FSubType74());
		break;

	case ESubType::SubType75:
		SubjectRecord.SetTrait(FSubType75());
		break;

	case ESubType::SubType76:
		SubjectRecord.SetTrait(FSubType76());
		break;

	case ESubType::SubType77:
		SubjectRecord.SetTrait(FSubType77());
		break;

	case ESubType::SubType78:
		SubjectRecord.SetTrait(FSubType78());
		break;

	case ESubType::SubType79:
		SubjectRecord.SetTrait(FSubType79());
		break;

	case ESubType::SubType80:
		SubjectRecord.SetTrait(FSubType80());
		break;

	case ESubType::SubType81:
		SubjectRecord.SetTrait(FSubType81());
		break;
	}
}

void UBattleFrameFunctionLibraryRT::SetSubjectSubTypeTraitByIndex(int32 Index, FSubjectHandle SubjectHandle)
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("SetRecordSubTypeTraitByIndex");
	switch (Index)
	{
	default:
		SubjectHandle.SetTrait(FSubType0());
		break;

	case 0:
		SubjectHandle.SetTrait(FSubType0());
		break;

	case 1:
		SubjectHandle.SetTrait(FSubType1());
		break;

	case 2:
		SubjectHandle.SetTrait(FSubType2());
		break;

	case 3:
		SubjectHandle.SetTrait(FSubType3());
		break;

	case 4:
		SubjectHandle.SetTrait(FSubType4());
		break;

	case 5:
		SubjectHandle.SetTrait(FSubType5());
		break;

	case 6:
		SubjectHandle.SetTrait(FSubType6());
		break;

	case 7:
		SubjectHandle.SetTrait(FSubType7());
		break;

	case 8:
		SubjectHandle.SetTrait(FSubType8());
		break;

	case 9:
		SubjectHandle.SetTrait(FSubType9());
		break;

	case 10:
		SubjectHandle.SetTrait(FSubType10());
		break;

	case 11:
		SubjectHandle.SetTrait(FSubType11());
		break;

	case 12:
		SubjectHandle.SetTrait(FSubType12());
		break;

	case 13:
		SubjectHandle.SetTrait(FSubType13());
		break;

	case 14:
		SubjectHandle.SetTrait(FSubType14());
		break;

	case 15:
		SubjectHandle.SetTrait(FSubType15());
		break;

	case 16:
		SubjectHandle.SetTrait(FSubType16());
		break;

	case 17:
		SubjectHandle.SetTrait(FSubType17());
		break;

	case 18:
		SubjectHandle.SetTrait(FSubType18());
		break;

	case 19:
		SubjectHandle.SetTrait(FSubType19());
		break;

	case 20:
		SubjectHandle.SetTrait(FSubType20());
		break;

	case 21:
		SubjectHandle.SetTrait(FSubType21());
		break;

	case 22:
		SubjectHandle.SetTrait(FSubType22());
		break;

	case 23:
		SubjectHandle.SetTrait(FSubType23());
		break;

	case 24:
		SubjectHandle.SetTrait(FSubType24());
		break;

	case 25:
		SubjectHandle.SetTrait(FSubType25());
		break;

	case 26:
		SubjectHandle.SetTrait(FSubType26());
		break;

	case 27:
		SubjectHandle.SetTrait(FSubType27());
		break;

	case 28:
		SubjectHandle.SetTrait(FSubType28());
		break;

	case 29:
		SubjectHandle.SetTrait(FSubType29());
		break;

	case 30:
		SubjectHandle.SetTrait(FSubType30());
		break;

	case 31:
		SubjectHandle.SetTrait(FSubType31());
		break;

	case 32:
		SubjectHandle.SetTrait(FSubType32());
		break;

	case 33:
		SubjectHandle.SetTrait(FSubType33());
		break;

	case 34:
		SubjectHandle.SetTrait(FSubType34());
		break;

	case 35:
		SubjectHandle.SetTrait(FSubType35());
		break;

	case 36:
		SubjectHandle.SetTrait(FSubType36());
		break;

	case 37:
		SubjectHandle.SetTrait(FSubType37());
		break;

	case 38:
		SubjectHandle.SetTrait(FSubType38());
		break;

	case 39:
		SubjectHandle.SetTrait(FSubType39());
		break;

	case 40:
		SubjectHandle.SetTrait(FSubType40());
		break;

	case 41:
		SubjectHandle.SetTrait(FSubType41());
		break;

	case 42:
		SubjectHandle.SetTrait(FSubType42());
		break;

	case 43:
		SubjectHandle.SetTrait(FSubType43());
		break;

	case 44:
		SubjectHandle.SetTrait(FSubType44());
		break;

	case 45:
		SubjectHandle.SetTrait(FSubType45());
		break;

	case 46:
		SubjectHandle.SetTrait(FSubType46());
		break;

	case 47:
		SubjectHandle.SetTrait(FSubType47());
		break;

	case 48:
		SubjectHandle.SetTrait(FSubType48());
		break;

	case 49:
		SubjectHandle.SetTrait(FSubType49());
		break;

	case 50:
		SubjectHandle.SetTrait(FSubType50());
		break;

	case 51:
		SubjectHandle.SetTrait(FSubType51());
		break;

	case 52:
		SubjectHandle.SetTrait(FSubType52());
		break;

	case 53:
		SubjectHandle.SetTrait(FSubType53());
		break;

	case 54:
		SubjectHandle.SetTrait(FSubType54());
		break;

	case 55:
		SubjectHandle.SetTrait(FSubType55());
		break;

	case 56:
		SubjectHandle.SetTrait(FSubType56());
		break;

	case 57:
		SubjectHandle.SetTrait(FSubType57());
		break;

	case 58:
		SubjectHandle.SetTrait(FSubType58());
		break;

	case 59:
		SubjectHandle.SetTrait(FSubType59());
		break;

	case 60:
		SubjectHandle.SetTrait(FSubType60());
		break;

	case 61:
		SubjectHandle.SetTrait(FSubType61());
		break;

	case 62:
		SubjectHandle.SetTrait(FSubType62());
		break;

	case 63:
		SubjectHandle.SetTrait(FSubType63());
		break;

	case 64:
		SubjectHandle.SetTrait(FSubType64());
		break;

	case 65:
		SubjectHandle.SetTrait(FSubType65());
		break;

	case 66:
		SubjectHandle.SetTrait(FSubType66());
		break;

	case 67:
		SubjectHandle.SetTrait(FSubType67());
		break;

	case 68:
		SubjectHandle.SetTrait(FSubType68());
		break;

	case 69:
		SubjectHandle.SetTrait(FSubType69());
		break;

	case 70:
		SubjectHandle.SetTrait(FSubType70());
		break;

	case 71:
		SubjectHandle.SetTrait(FSubType71());
		break;

	case 72:
		SubjectHandle.SetTrait(FSubType72());
		break;

	case 73:
		SubjectHandle.SetTrait(FSubType73());
		break;

	case 74:
		SubjectHandle.SetTrait(FSubType74());
		break;

	case 75:
		SubjectHandle.SetTrait(FSubType75());
		break;

	case 76:
		SubjectHandle.SetTrait(FSubType76());
		break;

	case 77:
		SubjectHandle.SetTrait(FSubType77());
		break;

	case 78:
		SubjectHandle.SetTrait(FSubType78());
		break;

	case 79:
		SubjectHandle.SetTrait(FSubType79());
		break;

	case 80:
		SubjectHandle.SetTrait(FSubType80());
		break;

	case 81:
		SubjectHandle.SetTrait(FSubType81());
		break;
	}
}

void UBattleFrameFunctionLibraryRT::IncludeSubTypeTraitByIndex(int32 Index, FFilter& Filter)
{
	switch (Index)
	{
	case 0:
		Filter.Include<FSubType0>();
		break;

	case 1:
		Filter.Include<FSubType1>();
		break;

	case 2:
		Filter.Include<FSubType2>();
		break;

	case 3:
		Filter.Include<FSubType3>();
		break;

	case 4:
		Filter.Include<FSubType4>();
		break;

	case 5:
		Filter.Include<FSubType5>();
		break;

	case 6:
		Filter.Include<FSubType6>();
		break;

	case 7:
		Filter.Include<FSubType7>();
		break;

	case 8:
		Filter.Include<FSubType8>();
		break;

	case 9:
		Filter.Include<FSubType9>();
		break;

	case 10:
		Filter.Include<FSubType10>();
		break;

	case 11:
		Filter.Include<FSubType11>();
		break;

	case 12:
		Filter.Include<FSubType12>();
		break;

	case 13:
		Filter.Include<FSubType13>();
		break;

	case 14:
		Filter.Include<FSubType14>();
		break;

	case 15:
		Filter.Include<FSubType15>();
		break;

	case 16:
		Filter.Include<FSubType16>();
		break;

	case 17:
		Filter.Include<FSubType17>();
		break;

	case 18:
		Filter.Include<FSubType18>();
		break;

	case 19:
		Filter.Include<FSubType19>();
		break;

	case 20:
		Filter.Include<FSubType20>();
		break;

	case 21:
		Filter.Include<FSubType21>();
		break;

	case 22:
		Filter.Include<FSubType22>();
		break;

	case 23:
		Filter.Include<FSubType23>();
		break;

	case 24:
		Filter.Include<FSubType24>();
		break;

	case 25:
		Filter.Include<FSubType25>();
		break;

	case 26:
		Filter.Include<FSubType26>();
		break;

	case 27:
		Filter.Include<FSubType27>();
		break;

	case 28:
		Filter.Include<FSubType28>();
		break;

	case 29:
		Filter.Include<FSubType29>();
		break;

	case 30:
		Filter.Include<FSubType30>();
		break;

	case 31:
		Filter.Include<FSubType31>();
		break;

	case 32:
		Filter.Include<FSubType32>();
		break;

	case 33:
		Filter.Include<FSubType33>();
		break;

	case 34:
		Filter.Include<FSubType34>();
		break;

	case 35:
		Filter.Include<FSubType35>();
		break;

	case 36:
		Filter.Include<FSubType36>();
		break;

	case 37:
		Filter.Include<FSubType37>();
		break;

	case 38:
		Filter.Include<FSubType38>();
		break;

	case 39:
		Filter.Include<FSubType39>();
		break;

	case 40:
		Filter.Include<FSubType40>();
		break;

	case 41:
		Filter.Include<FSubType41>();
		break;

	case 42:
		Filter.Include<FSubType42>();
		break;

	case 43:
		Filter.Include<FSubType43>();
		break;

	case 44:
		Filter.Include<FSubType44>();
		break;

	case 45:
		Filter.Include<FSubType45>();
		break;

	case 46:
		Filter.Include<FSubType46>();
		break;

	case 47:
		Filter.Include<FSubType47>();
		break;

	case 48:
		Filter.Include<FSubType48>();
		break;

	case 49:
		Filter.Include<FSubType49>();
		break;

	case 50:
		Filter.Include<FSubType50>();
		break;

	case 51:
		Filter.Include<FSubType51>();
		break;

	case 52:
		Filter.Include<FSubType52>();
		break;

	case 53:
		Filter.Include<FSubType53>();
		break;

	case 54:
		Filter.Include<FSubType54>();
		break;

	case 55:
		Filter.Include<FSubType55>();
		break;

	case 56:
		Filter.Include<FSubType56>();
		break;

	case 57:
		Filter.Include<FSubType57>();
		break;

	case 58:
		Filter.Include<FSubType58>();
		break;

	case 59:
		Filter.Include<FSubType59>();
		break;

	case 60:
		Filter.Include<FSubType60>();
		break;

	case 61:
		Filter.Include<FSubType61>();
		break;

	case 62:
		Filter.Include<FSubType62>();
		break;

	case 63:
		Filter.Include<FSubType63>();
		break;

	case 64:
		Filter.Include<FSubType64>();
		break;

	case 65:
		Filter.Include<FSubType65>();
		break;

	case 66:
		Filter.Include<FSubType66>();
		break;

	case 67:
		Filter.Include<FSubType67>();
		break;

	case 68:
		Filter.Include<FSubType68>();
		break;

	case 69:
		Filter.Include<FSubType69>();
		break;

	case 70:
		Filter.Include<FSubType70>();
		break;

	case 71:
		Filter.Include<FSubType71>();
		break;

	case 72:
		Filter.Include<FSubType72>();
		break;

	case 73:
		Filter.Include<FSubType73>();
		break;

	case 74:
		Filter.Include<FSubType74>();
		break;

	case 75:
		Filter.Include<FSubType75>();
		break;

	case 76:
		Filter.Include<FSubType76>();
		break;

	case 77:
		Filter.Include<FSubType77>();
		break;

	case 78:
		Filter.Include<FSubType78>();
		break;

	case 79:
		Filter.Include<FSubType79>();
		break;

	case 80:
		Filter.Include<FSubType80>();
		break;

	case 81:
		Filter.Include<FSubType81>();
		break;

	default:
		Filter.Include<FSubType0>();
		break;
	}
}

void UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(int32 IterableNum, int32 MaxThreadsAllowed, int32 MinBatchSizeAllowed, int32& ThreadsCount, int32& BatchSize)
{
	// 计算最大可能线程数（考虑最小批次限制）
	const int32 MaxPossibleThreads = FMath::Clamp(IterableNum / FMath::Max(1, MinBatchSizeAllowed), 1, MaxThreadsAllowed);

	// 最终确定使用的线程数
	ThreadsCount = FMath::Clamp(MaxPossibleThreads, 1, MaxThreadsAllowed);

	// 计算批次大小（使用向上取整算法解决余数问题）
	BatchSize = IterableNum / ThreadsCount;

	if (IterableNum % ThreadsCount != 0) 
	{
		BatchSize += 1;
	}

	// 最终限制批次大小范围
	BatchSize = FMath::Clamp(BatchSize, 1, FLT_MAX);
}

void UBattleFrameFunctionLibraryRT::SetSubjectAvoGroupTraitByIndex(int32 Index, FSubjectHandle SubjectHandle)
{
	switch (Index)
	{
	default:
		SubjectHandle.SetTrait(FAvoGroup0());
		break;
	case 0:
		SubjectHandle.SetTrait(FAvoGroup0());
		break;
	case 1:
		SubjectHandle.SetTrait(FAvoGroup1());
		break;
	case 2:
		SubjectHandle.SetTrait(FAvoGroup2());
		break;
	case 3:
		SubjectHandle.SetTrait(FAvoGroup3());
		break;
	case 4:
		SubjectHandle.SetTrait(FAvoGroup4());
		break;
	case 5:
		SubjectHandle.SetTrait(FAvoGroup5());
		break;
	case 6:
		SubjectHandle.SetTrait(FAvoGroup6());
		break;
	case 7:
		SubjectHandle.SetTrait(FAvoGroup7());
		break;
	case 8:
		SubjectHandle.SetTrait(FAvoGroup8());
		break;
	case 9:
		SubjectHandle.SetTrait(FAvoGroup9());
		break;
	}
}

void UBattleFrameFunctionLibraryRT::SetSubjectTeamTraitByIndex(int32 Index, FSubjectHandle SubjectHandle)
{
	switch (Index)
	{
	default:
		SubjectHandle.SetTrait(FTeam0());
		break;
	case 0:
		SubjectHandle.SetTrait(FTeam0());
		break;
	case 1:
		SubjectHandle.SetTrait(FTeam1());
		break;
	case 2:
		SubjectHandle.SetTrait(FTeam2());
		break;
	case 3:
		SubjectHandle.SetTrait(FTeam3());
		break;
	case 4:
		SubjectHandle.SetTrait(FTeam4());
		break;
	case 5:
		SubjectHandle.SetTrait(FTeam5());
		break;
	case 6:
		SubjectHandle.SetTrait(FTeam6());
		break;
	case 7:
		SubjectHandle.SetTrait(FTeam7());
		break;
	case 8:
		SubjectHandle.SetTrait(FTeam8());
		break;
	case 9:
		SubjectHandle.SetTrait(FTeam9());
		break;
	}
}

void UBattleFrameFunctionLibraryRT::SetRecordAvoGroupTraitByIndex(int32 Index, FSubjectRecord& SubjectRecord)
{
	switch (Index)
	{
	default:
		SubjectRecord.SetTrait(FAvoGroup0());
		break;
	case 0:
		SubjectRecord.SetTrait(FAvoGroup0());
		break;
	case 1:
		SubjectRecord.SetTrait(FAvoGroup1());
		break;
	case 2:
		SubjectRecord.SetTrait(FAvoGroup2());
		break;
	case 3:
		SubjectRecord.SetTrait(FAvoGroup3());
		break;
	case 4:
		SubjectRecord.SetTrait(FAvoGroup4());
		break;
	case 5:
		SubjectRecord.SetTrait(FAvoGroup5());
		break;
	case 6:
		SubjectRecord.SetTrait(FAvoGroup6());
		break;
	case 7:
		SubjectRecord.SetTrait(FAvoGroup7());
		break;
	case 8:
		SubjectRecord.SetTrait(FAvoGroup8());
		break;
	case 9:
		SubjectRecord.SetTrait(FAvoGroup9());
		break;
	}
}

void UBattleFrameFunctionLibraryRT::SetRecordTeamTraitByIndex(int32 Index, FSubjectRecord& SubjectRecord)
{
	switch (Index)
	{
	default:
		SubjectRecord.SetTrait(FTeam0());
		break;
	case 0:
		SubjectRecord.SetTrait(FTeam0());
		break;
	case 1:
		SubjectRecord.SetTrait(FTeam1());
		break;
	case 2:
		SubjectRecord.SetTrait(FTeam2());
		break;
	case 3:
		SubjectRecord.SetTrait(FTeam3());
		break;
	case 4:
		SubjectRecord.SetTrait(FTeam4());
		break;
	case 5:
		SubjectRecord.SetTrait(FTeam5());
		break;
	case 6:
		SubjectRecord.SetTrait(FTeam6());
		break;
	case 7:
		SubjectRecord.SetTrait(FTeam7());
		break;
	case 8:
		SubjectRecord.SetTrait(FTeam8());
		break;
	case 9:
		SubjectRecord.SetTrait(FTeam9());
		break;
	}
}

void UBattleFrameFunctionLibraryRT::IncludeAvoGroupTraitByIndex(int32 Index, FFilter& Filter)
{
	switch (Index)
	{
	case 0:
		Filter.Include<FAvoGroup0>();
		break;

	case 1:
		Filter.Include<FAvoGroup1>();
		break;

	case 2:
		Filter.Include<FAvoGroup2>();
		break;

	case 3:
		Filter.Include<FAvoGroup3>();
		break;

	case 4:
		Filter.Include<FAvoGroup4>();
		break;

	case 5:
		Filter.Include<FAvoGroup5>();
		break;

	case 6:
		Filter.Include<FAvoGroup6>();
		break;

	case 7:
		Filter.Include<FAvoGroup7>();
		break;

	case 8:
		Filter.Include<FAvoGroup8>();
		break;

	case 9:
		Filter.Include<FAvoGroup9>();
		break;
	}
}

void UBattleFrameFunctionLibraryRT::ExcludeAvoGroupTraitByIndex(int32 Index, FFilter& Filter)
{
	switch (Index)
	{
	case 0:
		Filter.Exclude<FAvoGroup0>();
		break;

	case 1:
		Filter.Exclude<FAvoGroup1>();
		break;

	case 2:
		Filter.Exclude<FAvoGroup2>();
		break;

	case 3:
		Filter.Exclude<FAvoGroup3>();
		break;

	case 4:
		Filter.Exclude<FAvoGroup4>();
		break;

	case 5:
		Filter.Exclude<FAvoGroup5>();
		break;

	case 6:
		Filter.Exclude<FAvoGroup6>();
		break;

	case 7:
		Filter.Exclude<FAvoGroup7>();
		break;

	case 8:
		Filter.Exclude<FAvoGroup8>();
		break;

	case 9:
		Filter.Exclude<FAvoGroup9>();
		break;
	}
}

FVector UBattleFrameFunctionLibraryRT::FindNewPatrolGoalLocation(const FPatrol& Patrol, const FCollider& Collider, const FTrace& Trace, const FLocated& Located, int32 MaxAttempts /*= 3*/)
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("FindNewPatrolGoalLocation");

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

TArray<FSubjectHandle> UBattleFrameFunctionLibraryRT::ConvertDmgResultsToSubjectHandles(const TArray<FDmgResult>& DmgResults)
{
	TArray<FSubjectHandle> SubjectHandles;
	SubjectHandles.Reserve(DmgResults.Num() * 2); // Reserve space for both Damaged and Instigator subjects

	for (const FDmgResult& Result : DmgResults)
	{
		// Add both DamagedSubject and InstigatorSubject to the array
		SubjectHandles.Add(Result.DamagedSubject);

		// Only add Instigator if it's valid (not empty)
		if (Result.InstigatorSubject.IsValid())
		{
			SubjectHandles.Add(Result.InstigatorSubject);
		}
	}

	return SubjectHandles;
}

TArray<FSubjectHandle> UBattleFrameFunctionLibraryRT::ConvertTraceResultsToSubjectHandles(const TArray<FTraceResult>& TraceResults)
{
	TArray<FSubjectHandle> SubjectHandles;
	SubjectHandles.Reserve(TraceResults.Num());

	for (const FTraceResult& Result : TraceResults)
	{
		SubjectHandles.Add(Result.Subject);
	}

	return SubjectHandles;
}





