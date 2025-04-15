/*
 * APPARATIST
 * Created: 2023-02-02 16:26:26
 * Author: Vladislav Dmitrievich Turbanov (vladislav@turbanov.ru)
 * Community forums: https://talk.turbanov.ru
 * Copyright 2019 - 2023, SP Vladislav Dmitrievich Turbanov
 * Made in Russia, Moscow City, Chekhov City
 */

 /*
  * BattleFrame
  * Refactor: 2025
  * Author: Leroy Works
  */

#include "NeighborGridActor.h"
#include "Async/Async.h"

ANeighborGridActor* ANeighborGridActor::Instance = nullptr;

ANeighborGridActor::ANeighborGridActor()
{
	PrimaryActorTick.bCanEverTick = false;
	const auto SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	NeighborGridComponent = CreateDefaultSubobject<UNeighborGridComponent>("NeighborGridComponent");
	SceneComponent->Mobility = EComponentMobility::Static;
	RootComponent = SceneComponent;
}

void ANeighborGridActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (NeighborGridComponent)
    {
        // Notify the component that the actor has been constructed or modified+
        //UE_LOG(LogTemp, Log, TEXT("CallingInitializeComponent"));
        NeighborGridComponent->InitializeComponent();
    }
}

//-------------------------------Async Trace-------------------------------

USphereSweepForSubjectsAsyncAction* USphereSweepForSubjectsAsyncAction::SphereSweepForSubjectsAsync(const UObject* WorldContextObject, ANeighborGridActor* NeighborGridActor, const FVector Start = FVector::ZeroVector, const FVector End = FVector::ZeroVector, float Radius = 0, const EAsyncSortMode SortMode = EAsyncSortMode::None, const FVector SortOrigin = FVector::ZeroVector, const int32 MaxCount = 1, const FFilter Filter = FFilter())
{
	USphereSweepForSubjectsAsyncAction* AsyncAction = NewObject<USphereSweepForSubjectsAsyncAction>();
	AsyncAction->RegisterWithGameInstance(WorldContextObject ? WorldContextObject->GetWorld() : nullptr);

	AsyncAction->NeighborGridActor = NeighborGridActor;
	AsyncAction->Start = Start;
	AsyncAction->End = End;
	AsyncAction->Radius = Radius;
	AsyncAction->SortMode = SortMode;
	AsyncAction->SortOrigin = SortOrigin;
	AsyncAction->MaxCount = MaxCount;
	AsyncAction->Filter = Filter;

	return AsyncAction;
}

void USphereSweepForSubjectsAsyncAction::Activate()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("SphereSweepForSubjectsAsync");

		TWeakObjectPtr<UNeighborGridComponent> NeighborGrid = NeighborGridActor->FindComponentByClass<UNeighborGridComponent>();
		TArray<FIntVector> CellCoords = NeighborGrid->GetGridCellsForCapsule(Start, End, Radius);

		const FVector TraceDir = (End - Start).GetSafeNormal();
		const float TraceLength = FVector::Distance(Start, End);

		// 检查每个单元中的subject
		for (FIntVector CellCoord : CellCoords)
		{
			const FNeighborGridCell& CageCell = NeighborGrid->Cells[NeighborGrid->GetIndexAt(CellCoord)];

			if (!CageCell.SubjectFingerprint.Matches(Filter.GetFingerprint())) continue;

			for (const FAvoiding& Data : CageCell.Subjects)
			{
				const FSubjectHandle Subject = Data.SubjectHandle;

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

				// 创建FTraceResult并添加到结果数组
				FTraceResult Result{ Subject ,SubjectPos ,FVector::DistSquared(SortOrigin, SubjectPos) };
				TempResults.Add(Result);
			}
		} 

		// 排序逻辑（在后台线程执行）
		if (SortMode != EAsyncSortMode::None)
		{
			TempResults.Sort([this](const FTraceResult& A, const FTraceResult& B) 
			{
				if (SortMode == EAsyncSortMode::NearToFar)
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
			TRACE_CPUPROFILER_EVENT_SCOPE_STR("SphereSweepForSubjectsSync");
			Results.Reset();

			int32 ValidCount = 0;
			const bool bRequireLimit = (MaxCount > 0);

			// 按预排序顺序遍历，遇到有效项立即收集
			for (const FTraceResult& TempResult : TempResults)
			{
				if (!TempResult.Subject.Matches(Filter)) continue;// this can only run on gamethread

				Results.Add(TempResult);
				ValidCount++;

				// 达到数量限制立即终止
				if (bRequireLimit && ValidCount >= MaxCount)
				{
					break;
				}
			}

			Completed.Broadcast(Results);
			SetReadyToDestroy();
		});
	});
}
