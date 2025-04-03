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

#include "NeighborGridComponent.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

#include "Traits/RegisterMultiple.h"
#include "Traits/Collider.h"
#include "Traits/Located.h"
#include "Traits/BoxObstacle.h"
#include "Traits/Moving.h"
#include "Traits/SphereObstacle.h"
#include "Traits/Appearing.h"
#include "Traits/Move.h"
#include "Traits/Trace.h"
#include "Traits/Avoiding.h"
#include "Traits/Corpse.h"
#include "Math/Vector2D.h"
#include "RVODefinitions.h"
#include "BattleFrameFunctionLibraryRT.h"


UNeighborGridComponent::UNeighborGridComponent()
{
	bWantsInitializeComponent = true;
}

void UNeighborGridComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UNeighborGridComponent::BeginDestroy()
{
	Super::BeginDestroy();
}

void UNeighborGridComponent::InitializeComponent()
{
	UE_LOG(LogTemp, Log, TEXT("InitializeComponentCalled"));

	DoInitializeCells();
	GetBounds();
	InvCellSizeCache = 1 / CellSize;
}

//--------------------------------------------Tracing----------------------------------------------------------------

// Get overlapping subjects
void UNeighborGridComponent::SphereTraceForSubjects(const FVector Origin, float Radius, const FFilter Filter, TArray<FSubjectHandle>& Results) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("SphereTraceForSubjects");

	TSet<FSubjectHandle> OverlappingSubjects;

	const FVector Range(Radius);
	const FIntVector CagePosMin = WorldToCage(Origin - Range);
	const FIntVector CagePosMax = WorldToCage(Origin + Range);

	// Precompute the filter fingerprint
	const FFingerprint FilterFingerprint = Filter.GetFingerprint();

	for (int32 i = CagePosMin.Z; i <= CagePosMax.Z; ++i)
	{
		for (int32 j = CagePosMin.Y; j <= CagePosMax.Y; ++j)
		{
			for (int32 k = CagePosMin.X; k <= CagePosMax.X; ++k)
			{
				const FIntVector NeighbourCellPos(k, j, i);

				if (LIKELY(IsInside(NeighbourCellPos)))
				{
					const auto& NeighbourCell = At(NeighbourCellPos);

					// Early out if the cell doesn't match the filter
					if (!NeighbourCell.SubjectFingerprint.Matches(FilterFingerprint)) continue;

					// Iterate over subjects in the cell
					for (const FAvoiding& Data : NeighbourCell.Subjects)
					{
						const FSubjectHandle OtherSubject = Data.SubjectHandle;

						if (LIKELY(OtherSubject.Matches(Filter)))
						{
							const FVector Delta = Origin - Data.Location;
							const float DistanceSqr = Delta.SizeSquared();

							float OtherRadius = Data.Radius;

							const float CombinedRadius = Radius + OtherRadius;
							const float CombinedRadiusSquared = FMath::Square(CombinedRadius);

							if (CombinedRadiusSquared > DistanceSqr)
							{
								OverlappingSubjects.Add(OtherSubject);
							}
						}
					}
				}
			}
		}
	}

	// 将结果转换为数组
	Results = OverlappingSubjects.Array();

}

void UNeighborGridComponent::SphereSweepForSubjects(const FVector Start, const FVector End, float Radius, const FFilter Filter, TArray<FSubjectHandle>& Results)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("SphereSweepForSubjects");

	TSet<FSubjectHandle> HitSubjects;
	TArray<FIntVector> GridCells = GetGridCellsForCapsule(Start, End, Radius);
	TSet<int32> VisitedCells;

	for (const FIntVector& CellCoord : GridCells)
	{
		if (IsInside(CellCoord))
		{
			VisitedCells.Add(GetIndexAt(CellCoord.X, CellCoord.Y, CellCoord.Z));
		}
	}

	const FVector TraceDir = (End - Start).GetSafeNormal();
	const float TraceLength = FVector::Distance(Start, End);

	for (int32 CellIndex : VisitedCells)
	{
		const FNeighborGridCell& CageCell = Cells[CellIndex];
		if (!CageCell.SubjectFingerprint.Matches(Filter.GetFingerprint())) continue;

		for (const FAvoiding& Data : CageCell.Subjects)
		{
			const FSubjectHandle Subject = Data.SubjectHandle;

			if (!Subject.IsValid() || !Subject.Matches(Filter)) continue;

			const FVector SubjectPos = Data.Location;

			float SubjectRadius = Data.Radius;

			const FVector ToSubject = SubjectPos - Start;
			const float ProjOnTrace = FVector::DotProduct(ToSubject, TraceDir);

			const float ProjThreshold = SubjectRadius + Radius;
			if (ProjOnTrace < -ProjThreshold || ProjOnTrace > TraceLength + ProjThreshold) continue;

			const float ClampedProj = FMath::Clamp(ProjOnTrace, 0.0f, TraceLength);
			const FVector NearestPoint = Start + ClampedProj * TraceDir;
			const float CombinedRadSq = FMath::Square(Radius + SubjectRadius);

			if (FVector::DistSquared(NearestPoint, SubjectPos) < CombinedRadSq)
			{
				HitSubjects.Add(Subject);
			}
		}
	}

	// 将结果转换为数组
	Results = HitSubjects.Array();
}

// for agents to trace nearest targets( cheaper this way)
void UNeighborGridComponent::SphereExpandForSubject(const FVector Origin, float Radius, float Height, const FFilter Filter, FSubjectHandle& Result) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("SphereExpandForSubjects");

	float ClosestDistanceSqr = FLT_MAX;
	FSubjectHandle ClosestSubject;

	const FVector Range(Radius, Radius, Height / 2.0f);
	const FIntVector CagePosMin = WorldToCage(Origin - Range);
	const FIntVector CagePosMax = WorldToCage(Origin + Range);

	// Precompute the filter fingerprint
	const FFingerprint FilterFingerprint = Filter.GetFingerprint();

	// Iterate over the z-axis layers
	for (int32 i = CagePosMin.Z; i <= CagePosMax.Z; ++i)
	{
		// Iterate over the xy plane, expanding outward in a circular pattern
		for (float CurrentRadius = 0.0f; CurrentRadius <= Radius; CurrentRadius += CellSize)
		{
			for (float Angle = 0.0f; Angle < 360.0f; Angle += 10.0f) // Adjust angle step for precision
			{
				float X = Origin.X + CurrentRadius * FMath::Cos(FMath::DegreesToRadians(Angle));
				float Y = Origin.Y + CurrentRadius * FMath::Sin(FMath::DegreesToRadians(Angle));

				FVector CurrentLocation(X, Y, Origin.Z);

				const FIntVector NeighbourCellPos = WorldToCage(CurrentLocation);

				if (LIKELY(IsInside(NeighbourCellPos)))
				{
					const auto& NeighbourCell = At(NeighbourCellPos);

					// Early out if the cell doesn't match the filter
					if (!NeighbourCell.SubjectFingerprint.Matches(FilterFingerprint)) continue;

					// Iterate over subjects in the cell
					for (const FAvoiding& Data : NeighbourCell.Subjects)
					{
						FSubjectHandle OtherSubject = Data.SubjectHandle;

						if (LIKELY(OtherSubject.Matches(Filter)))
						{
							const FVector Delta = Origin - Data.Location;
							const float DistanceSqr = Delta.SizeSquared();

							const float CombinedRadius = Radius + Data.Radius;
							const float CombinedRadiusSquared = FMath::Square(CombinedRadius);

							if (CombinedRadiusSquared > DistanceSqr)
							{
								if (DistanceSqr < ClosestDistanceSqr)
								{
									ClosestDistanceSqr = DistanceSqr;
									ClosestSubject = OtherSubject;
								}
							}
						}
					}
				}
			}
		}
	}

	// Set the output result
	Result = ClosestSubject.IsValid() ? ClosestSubject : FSubjectHandle();
}


//--------------------------------------------Avoidance---------------------------------------------------------------

// Performance Test Result : 5.5 ms per 10000 agents on AMD Ryzen 5900X.

void UNeighborGridComponent::Update()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("RVO2 Update");

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("ResetCells");

		ParallelFor(Cells.Num(), [&](int32 Index) // To do : only process occupied cells not all cells
		{
			FNeighborGridCell& Cell = Cells[Index];
			Cell.SubjectFingerprint.Reset();
			Cell.SphereObstacleFingerprint.Reset();
			Cell.BoxObstacleFingerprint.Reset();
			Cell.Subjects.Reset();
			Cell.SphereObstacles.Reset();
			Cell.BoxObstacles.Reset();
		});
	}

	AMechanism* Mechanism = GetMechanism();

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("RegisterNeighborGrid_Trace");

		FFilter Filter = FFilter::Make<FLocated, FTrace>();
		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently([&](FLocated& Located, FTrace& Trace)
		{
			const auto Location = Located.Location;

			if (UNLIKELY(!IsInside(Location))) return;

			Trace.Lock();
			Trace.NeighborGrid = this;// when agent traces, it uses this neighbor grid instance. in future i will add multiple neighbor support
			Trace.Unlock();

		}, ThreadsCount, BatchSize);
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("RegisterNeighborGrid_SphereObstacle");

		FFilter Filter = FFilter::Make<FLocated, FSphereObstacle>();
		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently([&](FLocated& Located, FSphereObstacle& SphereObstacle)
		{
			const auto Location = Located.Location;

			// Distance from the world origin check
			if (UNLIKELY(!IsInside(Location))) return;

			SphereObstacle.Lock();
			SphereObstacle.NeighborGrid = this;// when sphere obstacle override speed limit, it uses this neighbor grid instance.
			SphereObstacle.Unlock();

		}, ThreadsCount, BatchSize);
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("RegisterSubjectSingle");// agents are allowed to register themselves only in the cell where their origins are in, this helps to improve performance

		FFilter Filter = FFilter::Make<FLocated, FCollider, FAvoiding>().Exclude<FRegisterMultiple>();
		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently([&](FSolidSubjectHandle Subject, FLocated& Located, FCollider& Collider, FAvoiding& Avoiding)
		{
			const auto Location = Located.Location;

			if (UNLIKELY(!IsInside(Location))) return;

			Avoiding.Location = Location;
			Avoiding.Radius = Collider.Radius;

			auto& Cell = At(WorldToCage(Location));

			Cell.Lock();
			Cell.SubjectFingerprint.Add(Subject.GetFingerprint());
			Cell.Subjects.Add(Avoiding);
			Cell.Unlock();

		}, ThreadsCount, BatchSize);
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("RegisterSubjectMultiple");// agents can also register themselves into all overlapping cells, thus improve avoidance precision

		FFilter Filter = FFilter::Make<FLocated, FCollider, FAvoiding, FRegisterMultiple>().Exclude<FSphereObstacle>();
		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently([&](FSolidSubjectHandle Subject, FLocated& Located, FCollider& Collider, FAvoiding& Avoiding)
		{
			const auto Location = Located.Location;

			if (UNLIKELY(!IsInside(Location))) return;

			Avoiding.Location = Location;
			Avoiding.Radius = Collider.Radius;

			const FVector Range = FVector(Collider.Radius);

			// Compute the range of involved grid cells
			const FIntVector CagePosMin = WorldToCage(Location - Range);
			const FIntVector CagePosMax = WorldToCage(Location + Range);

			for (int32 i = CagePosMin.Z; i <= CagePosMax.Z; ++i)
			{
				for (int32 j = CagePosMin.Y; j <= CagePosMax.Y; ++j)
				{
					for (int32 k = CagePosMin.X; k <= CagePosMax.X; ++k)
					{
						const auto CurrentCellPos = FIntVector(k, j, i);

						if (!IsInside(CurrentCellPos)) continue;

						auto& Cell = At(CurrentCellPos);

						Cell.Lock();
						Cell.SubjectFingerprint.Add(Subject.GetFingerprint());
						Cell.Subjects.Add(Avoiding);
						Cell.Unlock();
					}
				}
			}

		}, ThreadsCount, BatchSize);
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("RegisterSphereObstacles");

		FFilter Filter = FFilter::Make<FLocated, FCollider, FAvoiding, FRegisterMultiple, FSphereObstacle>();
		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently([&](FSolidSubjectHandle Subject, FLocated& Located, FCollider& Collider, FAvoiding& Avoiding)
			{
				const auto Location = Located.Location;

				if (UNLIKELY(!IsInside(Location))) return;

				Avoiding.Location = Location;
				Avoiding.Radius = Collider.Radius;

				const FVector Range = FVector(Collider.Radius);

				// Compute the range of involved grid cells
				const FIntVector CagePosMin = WorldToCage(Location - Range);
				const FIntVector CagePosMax = WorldToCage(Location + Range);

				for (int32 i = CagePosMin.Z; i <= CagePosMax.Z; ++i)
				{
					for (int32 j = CagePosMin.Y; j <= CagePosMax.Y; ++j)
					{
						for (int32 k = CagePosMin.X; k <= CagePosMax.X; ++k)
						{
							const auto CurrentCellPos = FIntVector(k, j, i);

							if (!IsInside(CurrentCellPos)) continue;

							auto& Cell = At(CurrentCellPos);

							Cell.Lock();
							Cell.SphereObstacleFingerprint.Add(Subject.GetFingerprint());
							Cell.SphereObstacles.Add(Avoiding);
							Cell.Unlock();
						}
					}
				}
			}, ThreadsCount, BatchSize);
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("RegisterBoxObstacles");

		FFilter Filter = FFilter::Make<FBoxObstacle, FLocated, FAvoiding>();
		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, 1, ThreadsCount, BatchSize);

		Chain->OperateConcurrently([&](FSolidSubjectHandle Subject, FBoxObstacle& BoxObstacle, FAvoiding& Avoiding)
		{
			const auto SelfLocation = BoxObstacle.point3d_;
			Avoiding.Location = SelfLocation;

			const FBoxObstacle* PreObstaclePtr = BoxObstacle.prevObstacle_.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();
			const FBoxObstacle* NextObstaclePtr = BoxObstacle.nextObstacle_.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();

			if (NextObstaclePtr == nullptr || PreObstaclePtr == nullptr) return;

			const FVector NextLocation = NextObstaclePtr->point3d_;
			const float ObstacleHeight = BoxObstacle.height_;

			if (IsInside(SelfLocation) && IsInside(NextLocation))
			{
				const float StartZ = SelfLocation.Z;
				const float EndZ = StartZ + ObstacleHeight;
				TSet<FIntVector> AllGridCells;
				TArray<FIntVector> AllGridCellsArray;

				float CurrentLayerZ = StartZ;

				while (CurrentLayerZ < EndZ)
				{
					const FVector LayerCurrentPoint = FVector(SelfLocation.X, SelfLocation.Y, CurrentLayerZ);
					const FVector LayerNextPoint = FVector(NextLocation.X, NextLocation.Y, CurrentLayerZ);
					auto LayerCells = GetGridCellsForCapsule(LayerCurrentPoint, LayerNextPoint, CellSize * 2);

					for (const auto& CellPos : LayerCells)
					{
						AllGridCells.Add(CellPos);
					}

					CurrentLayerZ += CellSize;
				}

				AllGridCellsArray = AllGridCells.Array();

				ParallelFor(AllGridCellsArray.Num(),
					[&](int32 Index)
					{
						const FIntVector& CellPos = AllGridCellsArray[Index];
						if (!LIKELY(IsInside(CellPos))) return;

						auto& Cell = At(CellPos);

						Cell.Lock();
						Cell.BoxObstacleFingerprint.Add(Subject.GetFingerprint());
						Cell.BoxObstacles.Add(Avoiding);
						Cell.Unlock();
					}
				);
			}
		}, ThreadsCount, BatchSize);
	}
}

void UNeighborGridComponent::Decouple()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("RVO2 Decouple");

	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	const FFilter SphereObstacleFilter = FFilter::Make<FSphereObstacle, FAvoiding, FAvoidance, FLocated, FCollider>();
	const FFingerprint SphereObstacleFingerprint = SphereObstacleFilter.GetFingerprint();

	const FFilter BoxObstacleFilter = FFilter::Make<FBoxObstacle, FAvoiding, FLocated>();
	const FFingerprint BoxObstacleFingerprint = BoxObstacleFilter.GetFingerprint();

	AMechanism* Mechanism = GetMechanism();
	const FFilter Filter = FFilter::Make<FLocated, FCollider, FMove, FMoving, FAvoidance, FAvoiding>().Exclude<FSphereObstacle, FAppearing>();

	auto Chain = Mechanism->EnchainSolid(Filter);
	UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

	Chain->OperateConcurrently([&](FSolidSubjectHandle Subject, FMove& Move, FLocated& Located, FCollider& Collider, FMoving& Moving, FAvoidance& Avoidance, FAvoiding& Avoiding)
	{
		if (UNLIKELY(!Move.bEnable)) return;

		const auto SelfLocation = Located.Location;
		const auto SelfRadius = Collider.Radius;
		const auto NeighborDist = Avoidance.NeighborDist;
		const float TotalRangeSqr = FMath::Square(SelfRadius + NeighborDist);
		const int32 MaxNeighbors = Avoidance.MaxNeighbors;

		TArray<FAvoiding> SubjectNeighbors;
		TArray<FAvoiding> SphereObstacleNeighbors;
		TArray<FAvoiding> BoxObstacleNeighbors;

		SubjectNeighbors.Reserve(MaxNeighbors);
		SphereObstacleNeighbors.Reserve(MaxNeighbors);
		BoxObstacleNeighbors.Reserve(MaxNeighbors);

		//--------------------------Collect Subject Neighbors And Sphere Obstacles--------------------------------

		FFilter SubjectFilter = FFilter::Make<FLocated, FCollider, FAvoidance, FAvoiding>().Exclude<FSphereObstacle, FCorpse>();

		// 碰撞组
		if (!Avoidance.IgnoreGroups.IsEmpty())
		{
			const int32 ClampedGroups = FMath::Clamp(Avoidance.IgnoreGroups.Num(), 0, 9);

			for (int32 i = 0; i < ClampedGroups; ++i)
			{
				UBattleFrameFunctionLibraryRT::ExcludeAvoGroupTraitByIndex(Avoidance.IgnoreGroups[i], SubjectFilter);
			}
		}

		const FFingerprint SubjectFingerprint = SubjectFilter.GetFingerprint();

		const FVector SubjectRange3D(NeighborDist + SelfRadius, NeighborDist + SelfRadius, SelfRadius);
		TArray<FIntVector> NeighbourCellCoords = GetNeighborCells(SelfLocation, SubjectRange3D);

		// 使用最大堆收集最近的SubjectNeighbors
		auto SubjectCompare = [&](const FAvoiding& A, const FAvoiding& B) {
			return FVector::DistSquared(SelfLocation, A.Location) > FVector::DistSquared(SelfLocation, B.Location);
			};

		TSet<uint32> SeenHashes;

		TSet<FAvoiding> ValidSphereObstacleNeighbors;
		ValidSphereObstacleNeighbors.Reserve(MaxNeighbors);

		for (const FIntVector& Coord : NeighbourCellCoords)
		{
			const auto& Cell = At(Coord);

			if (Cell.SubjectFingerprint.Matches(SubjectFingerprint))
			{
				for (const FAvoiding& AvoData : Cell.Subjects)
				{
					// 基础过滤条件
					if (UNLIKELY(!AvoData.SubjectHandle.Matches(SubjectFilter))) continue;
					if (AvoData.SubjectHash == Avoiding.SubjectHash) continue;

					// 距离检查
					const float DistSqr = FVector::DistSquared(SelfLocation, AvoData.Location);
					const float RadiusSqr = FMath::Square(AvoData.Radius) + TotalRangeSqr;
					if (UNLIKELY(DistSqr > RadiusSqr)) continue;

					// 高效去重（基于栈内存的TSet）
					if (SeenHashes.Contains(AvoData.SubjectHash)) continue;
					SeenHashes.Add(AvoData.SubjectHash);

					// 动态维护堆
					if (SubjectNeighbors.Num() < MaxNeighbors)
					{
						SubjectNeighbors.HeapPush(AvoData, SubjectCompare);
					}
					else
					{
						const float HeapTopDist = FVector::DistSquared(SelfLocation, SubjectNeighbors.HeapTop().Location);
						if (DistSqr < HeapTopDist)
						{
							// 弹出时同步移除哈希记录
							SeenHashes.Remove(SubjectNeighbors.HeapTop().SubjectHash);
							SubjectNeighbors.HeapPopDiscard(SubjectCompare);
							SubjectNeighbors.HeapPush(AvoData, SubjectCompare);
						}
					}
				}
			}

			// SphereObstacle
			if (Cell.SphereObstacleFingerprint.Matches(SphereObstacleFingerprint))
			{
				for (const FAvoiding& AvoData : Cell.SphereObstacles)
				{
					if (UNLIKELY(!AvoData.SubjectHandle.Matches(SphereObstacleFilter))) continue;

					const float DistSqr = FVector::DistSquared(SelfLocation, AvoData.Location);
					const float RadiusSqr = FMath::Square(AvoData.Radius) + TotalRangeSqr;

					if (UNLIKELY(DistSqr > RadiusSqr)) continue;

					ValidSphereObstacleNeighbors.Add(AvoData);
				}
			}
		}

		// 按距离升序排序后加入结果
		SubjectNeighbors.Sort([&](const FAvoiding& A, const FAvoiding& B) { return FVector::DistSquared(SelfLocation, A.Location) < FVector::DistSquared(SelfLocation, B.Location);});

		SphereObstacleNeighbors.Append(ValidSphereObstacleNeighbors.Array());


		//---------------------------Collect Box Obstacle Neighbors--------------------------------

		const float ObstacleRange = Avoidance.RVO_TimeHorizon_Obstacle * Avoidance.MaxSpeed + Avoidance.Radius;
		const FVector ObstacleRange3D(ObstacleRange, ObstacleRange, Avoidance.Radius);
		TArray<FIntVector> ObstacleCellCoords = GetNeighborCells(SelfLocation, ObstacleRange3D);

		for (const FIntVector& Coord : ObstacleCellCoords)
		{
			const auto& Cell = At(Coord);
			if (LIKELY(!Cell.BoxObstacleFingerprint.Matches(BoxObstacleFingerprint))) continue;

			for (FAvoiding AvoData : Cell.BoxObstacles)
			{
				const FSubjectHandle OtherObstacle = AvoData.SubjectHandle;

				if (UNLIKELY(!OtherObstacle.Matches(BoxObstacleFilter))) continue;

				const auto& ObstacleTrait = OtherObstacle.GetTrait<FBoxObstacle>();
				const FVector ObstaclePoint = ObstacleTrait.point3d_;
				const FBoxObstacle* NextObstaclePtr = ObstacleTrait.nextObstacle_.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();

				if (UNLIKELY(NextObstaclePtr == nullptr)) continue;

				const FVector NextPoint = NextObstaclePtr->point3d_;
				const float ObstacleHeight = ObstacleTrait.height_;

				const float ObstacleZMin = ObstaclePoint.Z;
				const float ObstacleZMax = ObstaclePoint.Z + ObstacleHeight;
				const float SubjectZMin = SelfLocation.Z - SelfRadius;
				const float SubjectZMax = SelfLocation.Z + SelfRadius;

				if (SubjectZMax < ObstacleZMin || SubjectZMin > ObstacleZMax) continue;

				RVO::Vector2 currentPos(Located.Location.X, Located.Location.Y);
				RVO::Vector2 obstacleStart(ObstaclePoint.X, ObstaclePoint.Y);
				RVO::Vector2 obstacleEnd(NextPoint.X, NextPoint.Y);

				float leftOfValue = RVO::leftOf(obstacleStart, obstacleEnd, currentPos);

				if (leftOfValue < 0.0f)
				{
					BoxObstacleNeighbors.Add(AvoData);
				}
			}
		}

		Located.preLocation = Located.Location;

		Avoidance.Radius = SelfRadius;
		Avoidance.Position = RVO::Vector2(SelfLocation.X, SelfLocation.Y);

		//----------------------------Try Avoid SubjectNeighbors---------------------------------

		Avoidance.MaxSpeed = Moving.DesiredVelocity.Size2D() * Moving.SpeedMult;
		Avoidance.DesiredVelocity = RVO::Vector2(Moving.DesiredVelocity.X, Moving.DesiredVelocity.Y);//copy into rvo trait
		Avoidance.CurrentVelocity = RVO::Vector2(Moving.CurrentVelocity.X, Moving.CurrentVelocity.Y);//copy into rvo trait

		TArray<FAvoiding> EmptyArray;
		ComputeNewVelocity(Avoidance, SubjectNeighbors, EmptyArray, DeltaTime);

		if (!Moving.bFalling && !(Moving.LaunchTimer > 0))
		{
			FVector AvoidingVelocity(Avoidance.AvoidingVelocity.x(), Avoidance.AvoidingVelocity.y(), 0);
			FVector CurrentVelocity(Avoidance.CurrentVelocity.x(), Avoidance.CurrentVelocity.y(), 0);
			FVector InterpedVelocity = FMath::VInterpTo(CurrentVelocity, AvoidingVelocity, DeltaTime, Move.Acceleration / 100);
			Moving.CurrentVelocity = FVector(InterpedVelocity.X, InterpedVelocity.Y, Moving.CurrentVelocity.Z); // velocity can only change so much because of inertial
		}

		//-------------------------------Blocked By Obstacles------------------------------------

		Avoidance.MaxSpeed = Moving.bPushedBack ? FMath::Max(Moving.CurrentVelocity.Size2D(), Moving.PushBackSpeedOverride) : Moving.CurrentVelocity.Size2D();
		Avoidance.DesiredVelocity = RVO::Vector2(Moving.CurrentVelocity.X, Moving.CurrentVelocity.Y);//copy into rvo trait
		Avoidance.CurrentVelocity = RVO::Vector2(Moving.CurrentVelocity.X, Moving.CurrentVelocity.Y);//copy into rvo trait

		ComputeNewVelocity(Avoidance, SphereObstacleNeighbors, BoxObstacleNeighbors, DeltaTime);

		Moving.CurrentVelocity = FVector(Avoidance.AvoidingVelocity.x(), Avoidance.AvoidingVelocity.y(), Moving.CurrentVelocity.Z);// since obstacles are hard, we set velocity directly without any interpolation

		//----------------------------------Set Location---------------------------------------

		Located.Location += Moving.CurrentVelocity * DeltaTime;

	}, ThreadsCount, BatchSize);
}

void UNeighborGridComponent::Evaluate()
{
	Update();
	Decouple();
}

//-------------------------------RVO2D Copyright 2023, EastFoxStudio. All Rights Reserved-------------------------------

void UNeighborGridComponent::ComputeNewVelocity(FAvoidance& Avoidance, TArray<FAvoiding>& SubjectNeighbors, TArray<FAvoiding>& ObstacleNeighbors, float TimeStep_)
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("computeNewVelocity");

	Avoidance.OrcaLines.clear();

	/* Create obstacle ORCA lines. */
	if (!ObstacleNeighbors.IsEmpty())
	{
		const float invTimeHorizonObst = 1.0f / Avoidance.RVO_TimeHorizon_Obstacle;

		for (const auto& Data : ObstacleNeighbors) 
		{
			FBoxObstacle* obstacle1 = Data.SubjectHandle.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();
			FBoxObstacle* obstacle2 = obstacle1->nextObstacle_.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();

			const RVO::Vector2 relativePosition1 = obstacle1->point_ - Avoidance.Position;
			const RVO::Vector2 relativePosition2 = obstacle2->point_ - Avoidance.Position;

			/*
			 * Check if velocity obstacle of obstacle is already taken care of by
			 * previously constructed obstacle ORCA lines.
			 */
			bool alreadyCovered = false;

			for (size_t j = 0; j < Avoidance.OrcaLines.size(); ++j) {
				if (RVO::det(invTimeHorizonObst * relativePosition1 - Avoidance.OrcaLines[j].point, Avoidance.OrcaLines[j].direction) - invTimeHorizonObst * Avoidance.Radius >= -RVO_EPSILON && det(invTimeHorizonObst * relativePosition2 - Avoidance.OrcaLines[j].point, Avoidance.OrcaLines[j].direction) - invTimeHorizonObst * Avoidance.Radius >= -RVO_EPSILON) {
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

			const float radiusSq = RVO::sqr(Avoidance.Radius);

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
				leftLegDirection = RVO::Vector2(relativePosition1.x() * leg1 - relativePosition1.y() * Avoidance.Radius, relativePosition1.x() * Avoidance.Radius + relativePosition1.y() * leg1) / distSq1;
				rightLegDirection = RVO::Vector2(relativePosition1.x() * leg1 + relativePosition1.y() * Avoidance.Radius, -relativePosition1.x() * Avoidance.Radius + relativePosition1.y() * leg1) / distSq1;
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
				leftLegDirection = RVO::Vector2(relativePosition2.x() * leg2 - relativePosition2.y() * Avoidance.Radius, relativePosition2.x() * Avoidance.Radius + relativePosition2.y() * leg2) / distSq2;
				rightLegDirection = RVO::Vector2(relativePosition2.x() * leg2 + relativePosition2.y() * Avoidance.Radius, -relativePosition2.x() * Avoidance.Radius + relativePosition2.y() * leg2) / distSq2;
			}
			else {
				/* Usual situation. */
				if (obstacle1->isConvex_) {
					const float leg1 = std::sqrt(distSq1 - radiusSq);
					leftLegDirection = RVO::Vector2(relativePosition1.x() * leg1 - relativePosition1.y() * Avoidance.Radius, relativePosition1.x() * Avoidance.Radius + relativePosition1.y() * leg1) / distSq1;
				}
				else {
					/* Left vertex non-convex; left leg extends cut-off line. */
					leftLegDirection = -obstacle1->unitDir_;
				}

				if (obstacle2->isConvex_) {
					const float leg2 = std::sqrt(distSq2 - radiusSq);
					rightLegDirection = RVO::Vector2(relativePosition2.x() * leg2 + relativePosition2.y() * Avoidance.Radius, -relativePosition2.x() * Avoidance.Radius + relativePosition2.y() * leg2) / distSq2;
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
			const RVO::Vector2 leftCutoff = invTimeHorizonObst * (obstacle1->point_ - Avoidance.Position);
			const RVO::Vector2 rightCutoff = invTimeHorizonObst * (obstacle2->point_ - Avoidance.Position);
			const RVO::Vector2 cutoffVec = rightCutoff - leftCutoff;

			/* Project current velocity on velocity obstacle. */

			/* Check if current velocity is projected on cutoff circles. */
			const float t = (obstacle1 == obstacle2 ? 0.5f : ((Avoidance.CurrentVelocity - leftCutoff) * cutoffVec) / absSq(cutoffVec));
			const float tLeft = ((Avoidance.CurrentVelocity - leftCutoff) * leftLegDirection);
			const float tRight = ((Avoidance.CurrentVelocity - rightCutoff) * rightLegDirection);

			if ((t < 0.0f && tLeft < 0.0f) || (obstacle1 == obstacle2 && tLeft < 0.0f && tRight < 0.0f)) {
				/* Project on left cut-off circle. */
				const RVO::Vector2 unitW = normalize(Avoidance.CurrentVelocity - leftCutoff);

				line.direction = RVO::Vector2(unitW.y(), -unitW.x());
				line.point = leftCutoff + Avoidance.Radius * invTimeHorizonObst * unitW;
				Avoidance.OrcaLines.push_back(line);
				continue;
			}
			else if (t > 1.0f && tRight < 0.0f) {
				/* Project on right cut-off circle. */
				const RVO::Vector2 unitW = normalize(Avoidance.CurrentVelocity - rightCutoff);

				line.direction = RVO::Vector2(unitW.y(), -unitW.x());
				line.point = rightCutoff + Avoidance.Radius * invTimeHorizonObst * unitW;
				Avoidance.OrcaLines.push_back(line);
				continue;
			}

			/*
			 * Project on left leg, right leg, or cut-off line, whichever is closest
			 * to velocity.
			 */
			const float distSqCutoff = ((t < 0.0f || t > 1.0f || obstacle1 == obstacle2) ? std::numeric_limits<float>::infinity() : absSq(Avoidance.CurrentVelocity - (leftCutoff + t * cutoffVec)));
			const float distSqLeft = ((tLeft < 0.0f) ? std::numeric_limits<float>::infinity() : absSq(Avoidance.CurrentVelocity - (leftCutoff + tLeft * leftLegDirection)));
			const float distSqRight = ((tRight < 0.0f) ? std::numeric_limits<float>::infinity() : absSq(Avoidance.CurrentVelocity - (rightCutoff + tRight * rightLegDirection)));

			if (distSqCutoff <= distSqLeft && distSqCutoff <= distSqRight) {
				/* Project on cut-off line. */
				line.direction = -obstacle1->unitDir_;
				line.point = leftCutoff + Avoidance.Radius * invTimeHorizonObst * RVO::Vector2(-line.direction.y(), line.direction.x());
				Avoidance.OrcaLines.push_back(line);
				continue;
			}
			else if (distSqLeft <= distSqRight) {
				/* Project on left leg. */
				if (isLeftLegForeign) {
					continue;
				}

				line.direction = leftLegDirection;
				line.point = leftCutoff + Avoidance.Radius * invTimeHorizonObst * RVO::Vector2(-line.direction.y(), line.direction.x());
				Avoidance.OrcaLines.push_back(line);
				continue;
			}
			else {
				/* Project on right leg. */
				if (isRightLegForeign) {
					continue;
				}

				line.direction = -rightLegDirection;
				line.point = rightCutoff + Avoidance.Radius * invTimeHorizonObst * RVO::Vector2(-line.direction.y(), line.direction.x());
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
			const auto& other = Data.SubjectHandle.GetTraitRef<FAvoidance, EParadigm::Unsafe>();
			const RVO::Vector2 relativePosition = other.Position - Avoidance.Position;
			const RVO::Vector2 relativeVelocity = Avoidance.CurrentVelocity - other.CurrentVelocity;
			const float distSq = absSq(relativePosition);
			const float combinedRadius = Avoidance.Radius + other.Radius;
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

			line.point = Avoidance.CurrentVelocity + 0.5f * u;
			Avoidance.OrcaLines.push_back(line);
		}
	}

	size_t lineFail = LinearProgram2(Avoidance.OrcaLines, Avoidance.MaxSpeed, Avoidance.DesiredVelocity, false, Avoidance.AvoidingVelocity);

	if (lineFail < Avoidance.OrcaLines.size()) {
		LinearProgram3(Avoidance.OrcaLines, numObstLines, lineFail, Avoidance.MaxSpeed, Avoidance.AvoidingVelocity);
	}
}

bool UNeighborGridComponent::LinearProgram1(const std::vector<RVO::Line>& lines, size_t lineNo, float radius, const RVO::Vector2& optVelocity, bool directionOpt, RVO::Vector2& result)
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

size_t UNeighborGridComponent::LinearProgram2(const std::vector<RVO::Line>& lines, float radius, const RVO::Vector2& optVelocity, bool directionOpt, RVO::Vector2& result)
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

void UNeighborGridComponent::LinearProgram3(const std::vector<RVO::Line>& lines, size_t numObstLines, size_t beginLine, float radius, RVO::Vector2& result)
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