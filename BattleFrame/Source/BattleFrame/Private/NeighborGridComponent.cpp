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
#include "Traits/Activated.h"
#include "Traits/Excluding.h"
#include "Math/Vector2D.h"
#include "RVODefinitions.h"
#include "Async/Async.h"
#include "BattleFrameFunctionLibraryRT.h"
#include "Traits/Agent.h"


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

// Multi Trace For Subjects
void UNeighborGridComponent::SphereTraceForSubjects(const FVector Origin, const float Radius, const bool bCheckVisibility, const FVector CheckOrigin, const float CheckRadius, const TArray<FSubjectHandle> IgnoreSubjects, const FFilter Filter, bool& Hit, TArray<FTraceResult>& Results) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("SphereTraceForSubjects");

	Results.Empty(); // 清空结果数组

	const FVector Range(Radius);
	const FIntVector CagePosMin = WorldToCage(Origin - Range);
	const FIntVector CagePosMax = WorldToCage(Origin + Range);

	// 预计算过滤器指纹
	const FFingerprint FilterFingerprint = Filter.GetFingerprint();

	// 将忽略列表转换为集合以便快速查找
	TSet<FSubjectHandle> IgnoreSet(IgnoreSubjects);

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

					// 如果单元格不匹配过滤器则跳过
					if (!NeighbourCell.SubjectFingerprint.Matches(FilterFingerprint)) continue;

					// 遍历单元格中的subjects
					for (const FAvoiding& Data : NeighbourCell.Subjects)
					{
						const FSubjectHandle OtherSubject = Data.SubjectHandle;

						// 检查是否在忽略列表中
						if (IgnoreSet.Contains(OtherSubject)) continue;

						if (LIKELY(OtherSubject.Matches(Filter)))
						{
							const FVector Delta = Origin - Data.Location;
							const float DistanceSqr = Delta.SizeSquared();

							float OtherRadius = Data.Radius;
							const float CombinedRadius = Radius + OtherRadius;
							const float CombinedRadiusSquared = FMath::Square(CombinedRadius);

							if (CombinedRadiusSquared > DistanceSqr)
							{
								if (bCheckVisibility)
								{
									bool bHit = false;
									FTraceResult VisibilityResult;

									// 计算目标球体表面点（考虑目标半径）
									const FVector ToSubjectDir = (Data.Location - CheckOrigin).GetSafeNormal();
									const FVector SubjectSurfacePoint = Data.Location - (ToSubjectDir * OtherRadius);

									SphereSweepForObstacle(CheckOrigin, SubjectSurfacePoint, CheckRadius, bHit, VisibilityResult);

									if (bHit) continue; // Path is blocked, skip this subject
								}

								// 创建FTraceResult并添加到结果数组
								FTraceResult Result;
								Result.Subject = OtherSubject;
								Result.Location = Data.Location;
								Result.CachedDistSq = FVector::DistSquared(Origin, Data.Location); // Precompute distance
								Results.Add(Result);
							}
						}
					}
				}
			}
		}
	}

	Hit = !Results.IsEmpty();
}

// Multi Sweep Trace For Subjects
void UNeighborGridComponent::SphereSweepForSubjects(const FVector Start, const FVector End, const float Radius, const bool bCheckVisibility, const FVector CheckOrigin, const float CheckRadius, const TArray<FSubjectHandle> IgnoreSubjects, const FFilter Filter, bool& Hit, TArray<FTraceResult>& Results)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("SphereSweepForSubjects");

	Results.Empty(); // Clear result array

	// Convert ignore list to set for fast lookup
	TSet<FSubjectHandle> IgnoreSet(IgnoreSubjects);

	TArray<FIntVector> GridCells = SphereSweepForCells(Start, End, Radius);

	const FVector TraceDir = (End - Start).GetSafeNormal();
	const float TraceLength = FVector::Distance(Start, End);

	// Check subjects in each cell
	for (const FIntVector& CellIndex : GridCells)
	{
		const FNeighborGridCell& CageCell = At(CellIndex.X, CellIndex.Y, CellIndex.Z);

		if (!CageCell.SubjectFingerprint.Matches(Filter.GetFingerprint())) continue;

		for (const FAvoiding& Data : CageCell.Subjects)
		{
			const FSubjectHandle Subject = Data.SubjectHandle;

			// Check if in ignore list
			if (IgnoreSet.Contains(Subject)) continue;

			// Validity checks
			if (!Subject.IsValid() || !Subject.Matches(Filter)) continue;

			const FVector SubjectPos = Data.Location;
			float SubjectRadius = Data.Radius;

			// Distance calculations
			const FVector ToSubject = SubjectPos - Start;
			const float ProjOnTrace = FVector::DotProduct(ToSubject, TraceDir);

			// Initial filtering
			const float ProjThreshold = SubjectRadius + Radius;
			if (ProjOnTrace < -ProjThreshold || ProjOnTrace > TraceLength + ProjThreshold) continue;

			// Precise distance check
			const float ClampedProj = FMath::Clamp(ProjOnTrace, 0.0f, TraceLength);
			const FVector NearestPoint = Start + ClampedProj * TraceDir;
			const float CombinedRadSq = FMath::Square(Radius + SubjectRadius);

			if (FVector::DistSquared(NearestPoint, SubjectPos) < CombinedRadSq)
			{
				if (bCheckVisibility)
				{
					// Perform visibility check
					bool bHit = false;
					FTraceResult VisibilityResult;

					// Calculate the surface point on the subject's sphere
					const FVector ToSubjectDir = (SubjectPos - CheckOrigin).GetSafeNormal();
					const FVector SubjectSurfacePoint = SubjectPos - (ToSubjectDir * SubjectRadius);

					SphereSweepForObstacle(CheckOrigin, SubjectSurfacePoint, CheckRadius, bHit, VisibilityResult);

					if (bHit) continue; // Path is blocked, skip this subject
				}

				// Create FTraceResult and add to results array
				FTraceResult Result;
				Result.Subject = Subject;
				Result.Location = SubjectPos;
				Result.CachedDistSq = FVector::DistSquared(Start, SubjectPos); // Precompute distance
				Results.Add(Result);
			}
		}
	}

	Hit = !Results.IsEmpty();
}

// Single Sector Trace For Nearest Subject
void UNeighborGridComponent::SectorTraceForSubject(const FVector Origin, const float Radius, const float Height, const FVector Direction, const float Angle, const bool bCheckVisibility, const FVector CheckOrigin, const float CheckRadius, const TArray<FSubjectHandle> IgnoreSubjects, const FFilter Filter, bool& Hit, FTraceResult& Result) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("SectorTraceForSubject");

	float ClosestDistanceSqr = FLT_MAX;
	FTraceResult ClosestResult;

	// 将忽略列表转换为集合以便快速查找
	TSet<FSubjectHandle> IgnoreSet(IgnoreSubjects);

	const FVector NormalizedDir = Direction.GetSafeNormal2D();
	const float HalfAngleRad = FMath::DegreesToRadians(Angle * 0.5f);
	const float CosHalfAngle = FMath::Cos(HalfAngleRad);

	// 扩展搜索范围以包含可能相交的格子
	const float CellSizeXY = CellSize; // 假设CellSize是成员变量，表示格子尺寸
	const float CellRadiusXY = CellSizeXY * 0.5f * FMath::Sqrt(2.0f); // 对角线缓冲
	const float ExpandedRadius = Radius + CellRadiusXY;
	const FVector Range(ExpandedRadius, ExpandedRadius, Height / 2.0f + CellSize * 0.5f); // 三维缓冲

	const FIntVector CagePosMin = WorldToCage(Origin - Range);
	const FIntVector CagePosMax = WorldToCage(Origin + Range);

	const FFingerprint FilterFingerprint = Filter.GetFingerprint();

	// 预收集候选格子（只做扩展距离检查）
	TArray<FIntVector> CandidateCells;
	for (int32 z = CagePosMin.Z; z <= CagePosMax.Z; ++z) {
		for (int32 x = CagePosMin.X; x <= CagePosMax.X; ++x) {
			for (int32 y = CagePosMin.Y; y <= CagePosMax.Y; ++y) {
				const FIntVector CellPos(x, y, z);
				if (IsInside(CellPos)) {
					const FVector CellCenter = CageToWorld(CellPos);

					// 扩展距离检查
					const FVector DeltaXY = (CellCenter - Origin) * FVector(1, 1, 0);
					if (DeltaXY.SizeSquared() > FMath::Square(ExpandedRadius)) continue;

					CandidateCells.Add(CellPos);
				}
			}
		}
	}

	// 按距离排序（使用原始半径排序）
	CandidateCells.Sort([&](const FIntVector& A, const FIntVector& B)
		{
			return FVector::DistSquared2D(CageToWorld(A), Origin) <
				FVector::DistSquared2D(CageToWorld(B), Origin);
		});

	// 遍历检测
	for (const FIntVector& CellPos : CandidateCells)
	{
		const auto& CellData = At(CellPos);
		if (!CellData.SubjectFingerprint.Matches(FilterFingerprint)) continue;

		// 提前终止检查
		const float CellDistSqr = FVector::DistSquared2D(CageToWorld(CellPos), Origin);
		if (CellDistSqr > ClosestDistanceSqr + FMath::Square(CellSize)) break;

		for (const FAvoiding& SubjectData : CellData.Subjects)
		{
			const FSubjectHandle Subject = SubjectData.SubjectHandle;

			// 检查是否在忽略列表中
			if (IgnoreSet.Contains(Subject)) continue;
			if (!Subject.Matches(Filter)) continue;

			// 精确距离检测（考虑目标半径）
			const FVector Delta = SubjectData.Location - Origin;
			const float DistanceSqr = Delta.SizeSquared();
			const float CombinedRadiusSq = FMath::Square(Radius + SubjectData.Radius);

			if (DistanceSqr > CombinedRadiusSq) continue;

			// 精确扇形检测
			const FVector DeltaXY = Delta * FVector(1, 1, 0);

			if (!DeltaXY.IsNearlyZero())
			{
				const FVector NormalizedDelta = DeltaXY.GetSafeNormal();
				const float CosTheta = FVector::DotProduct(NormalizedDelta, NormalizedDir);
				if (CosTheta < CosHalfAngle) continue;
			}

			// 更新最近目标 (原有逻辑)
			if (DistanceSqr < ClosestDistanceSqr)
			{
				/** 新增可见性检查逻辑 **/
				if (bCheckVisibility)
				{
					// 执行可见性检测
					bool HitObstacle;
					FTraceResult TraceResult;

					// 计算目标球体表面点（考虑目标半径）
					const FVector ToSubjectDir = (SubjectData.Location - CheckOrigin).GetSafeNormal();
					const FVector SubjectSurfacePoint = SubjectData.Location - (ToSubjectDir * SubjectData.Radius);

					SphereSweepForObstacle(CheckOrigin, SubjectSurfacePoint, CheckRadius, HitObstacle, TraceResult);

					if (HitObstacle) continue; // 路径被阻挡，跳过该目标
				}

				ClosestDistanceSqr = DistanceSqr;
				ClosestResult.Subject = Subject;
				ClosestResult.Location = SubjectData.Location;

				if (ClosestDistanceSqr <= KINDA_SMALL_NUMBER)
				{
					Result = ClosestResult;
					return;
				}
			}
		}
	}

	Hit = ClosestResult.Subject.IsValid();
	Result = Hit ? ClosestResult : FTraceResult();
}

// Single Sweep Trace For Nearest Obstacle
void UNeighborGridComponent::SphereSweepForObstacle(FVector Start, FVector End, float Radius, bool& Hit, FTraceResult& Result) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("SphereSweepForObstacle");

	Hit = false;
	Result = FTraceResult();
	float ClosestHitDistSq = FLT_MAX;

	TArray<FIntVector> PathCells = SphereSweepForCells(Start, End, Radius);

	auto ProcessObstacle = [&](const FAvoiding& Obstacle, float DistSqr)
	{
		if (DistSqr < ClosestHitDistSq)
		{
			ClosestHitDistSq = DistSqr;
			Hit = true;
			Result.Subject = Obstacle.SubjectHandle;
			Result.Location = Obstacle.Location;
			Result.CachedDistSq = DistSqr;
		}
	};

	for (const FIntVector& CellPos : PathCells)
	{
		const auto& Cell = At(CellPos);

		// 检查球形障碍物
		auto CheckSphereCollision = [&](const TArray<FAvoiding>& Obstacles)
		{
			for (const FAvoiding& Avoiding : Obstacles)
			{
				if (!Avoiding.SubjectHandle.IsValid()) continue;

				const FSphereObstacle* CurrentObstacle = Avoiding.SubjectHandle.GetTraitPtr<FSphereObstacle, EParadigm::Unsafe>();
				if (!CurrentObstacle) continue;
				if (CurrentObstacle->bExcluded) continue;

				// 计算球体到线段的最短距离平方
				const float DistSqr = FMath::PointDistToSegmentSquared(Avoiding.Location, Start, End);
				const float CombinedRadius = Radius + Avoiding.Radius;

				// 如果距离小于合并半径，则发生碰撞
				if (DistSqr <= FMath::Square(CombinedRadius))
				{
					ProcessObstacle(Avoiding, DistSqr);
				}
			}
		};

		// 检查静态/动态球形障碍物
		CheckSphereCollision(Cell.SphereObstacles);
		CheckSphereCollision(Cell.SphereObstaclesStatic);

		// 检查长方体障碍物碰撞
		auto CheckBoxCollision = [&](const TArray<FAvoiding>& Obstacles)
		{
			for (const FAvoiding& Avoiding : Obstacles)
			{
				if (!Avoiding.SubjectHandle.IsValid()) continue;

				const FBoxObstacle* CurrentObstacle = Avoiding.SubjectHandle.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();
				if (!CurrentObstacle) continue;
				if (CurrentObstacle->bExcluded) continue;

				const FBoxObstacle* PrevObstacle = CurrentObstacle->prevObstacle_.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();
				if (!PrevObstacle) continue;
				if (CurrentObstacle->bExcluded) continue;

				const FBoxObstacle* NextObstacle = CurrentObstacle->nextObstacle_.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();
				if (!NextObstacle) continue;
				if (CurrentObstacle->bExcluded) continue;

				const FBoxObstacle* NextNextObstacle = NextObstacle->nextObstacle_.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();
				if (!NextNextObstacle) continue;
				if (CurrentObstacle->bExcluded) continue;

				const FVector& CurrentPos = CurrentObstacle->point3d_;
				const FVector& PrevPos = PrevObstacle->point3d_;
				const FVector& NextPos = NextObstacle->point3d_;
				const FVector& NextNextPos = NextNextObstacle->point3d_;

				const float HalfHeight = CurrentObstacle->height_ * 0.5f;
				const FVector Up = FVector::UpVector;

				const FVector BottomVertices[4] =
				{
					PrevPos - Up * HalfHeight,
					CurrentPos - Up * HalfHeight,
					NextPos - Up * HalfHeight,
					NextNextPos - Up * HalfHeight
				};

				const FVector TopVertices[4] =
				{
					PrevPos + Up * HalfHeight,
					CurrentPos + Up * HalfHeight,
					NextPos + Up * HalfHeight,
					NextNextPos + Up * HalfHeight
				};

				auto SphereIntersectsBox = [](const FVector& SphereStart, const FVector& SphereEnd, float SphereRadius, const FVector BottomVerts[4], const FVector TopVerts[4]) -> bool
				{
					// 将球体运动轨迹视为胶囊体
					const FVector CapsuleDir = (SphereEnd - SphereStart).GetSafeNormal();
					const float CapsuleLength = FVector::Dist(SphereStart, SphereEnd);

					// 定义长方体的6个面
					const TArray<TArray<FVector>> Faces =
					{
						// 底面
						{BottomVerts[0], BottomVerts[1], BottomVerts[2], BottomVerts[3]},
						// 顶面
						{TopVerts[0], TopVerts[1], TopVerts[2], TopVerts[3]},
						// 侧面
						{BottomVerts[0], TopVerts[0], TopVerts[1], BottomVerts[1]},
						{BottomVerts[1], TopVerts[1], TopVerts[2], BottomVerts[2]},
						{BottomVerts[2], TopVerts[2], TopVerts[3], BottomVerts[3]},
						{BottomVerts[3], TopVerts[3], TopVerts[0], BottomVerts[0]}
					};

					// 检查每个面
					for (const auto& Face : Faces)
					{
						// 计算面法线
						const FVector Edge1 = Face[1] - Face[0];
						const FVector Edge2 = Face[2] - Face[0];
						FVector Normal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();

						// 计算平面方程: N·X + D = 0
						const float D = -FVector::DotProduct(Normal, Face[0]);

						// 计算球体线段到平面的距离
						const float DistStart = FVector::DotProduct(Normal, SphereStart) + D;
						const float DistEnd = FVector::DotProduct(Normal, SphereEnd) + D;

						// 检查是否与平面相交
						if (DistStart * DistEnd > 0 &&
							FMath::Abs(DistStart) > SphereRadius &&
							FMath::Abs(DistEnd) > SphereRadius)
						{
							continue; // 不相交
						}

						// 计算线段与平面的交点
						FVector Intersection;
						if (DistStart == DistEnd)
						{
							// 线段与平面平行
							continue;
						}
						else
						{
							const float t = -DistStart / (DistEnd - DistStart);
							Intersection = SphereStart + t * (SphereEnd - SphereStart);
						}

						// 手动实现点在多边形内的测试
						bool bInside = true;
						for (int i = 0; i < Face.Num(); ++i)
						{
							const FVector& CurrentVert = Face[i];
							const FVector& NextVert = Face[(i + 1) % Face.Num()];

							// 计算边向量
							const FVector Edge = NextVert - CurrentVert;
							const FVector ToPoint = Intersection - CurrentVert;

							// 使用叉积检查点是否在边的"内侧"
							if (FVector::DotProduct(FVector::CrossProduct(Edge, ToPoint), Normal) < 0)
							{
								bInside = false;
								break;
							}
						}

						if (bInside)
						{
							return true;
						}

						// 检查球体端点与面的距离
						auto PointToFaceDistance = [](const FVector& Point, const TArray<FVector>& Face, const FVector& Normal) -> float
							{
								TRACE_CPUPROFILER_EVENT_SCOPE_STR("PointToFaceDistance");

								// 计算点到平面的距离
								const float PlaneDist = FMath::Abs(FVector::DotProduct(Normal, Point - Face[0]));

								// 检查投影点是否在面内
								bool bInside = true;
								for (int i = 0; i < Face.Num(); ++i)
								{
									const FVector& CurrentVert = Face[i];
									const FVector& NextVert = Face[(i + 1) % Face.Num()];
									const FVector Edge = NextVert - CurrentVert;
									const FVector ToPoint = Point - CurrentVert;

									if (FVector::DotProduct(FVector::CrossProduct(Edge, ToPoint), Normal) < 0)
									{
										bInside = false;
										break;
									}
								}

								if (bInside)
								{
									return PlaneDist;
								}

								// 如果不在面内，计算到各边的最短距离
								float MinDist = FLT_MAX;
								for (int i = 0; i < Face.Num(); ++i)
								{
									const FVector& EdgeStart = Face[i];
									const FVector& EdgeEnd = Face[(i + 1) % Face.Num()];

									const FVector Edge = EdgeEnd - EdgeStart;
									const FVector ToPoint = Point - EdgeStart;

									const float EdgeLength = Edge.Size();
									const float t = FMath::Clamp(FVector::DotProduct(ToPoint, Edge) / (EdgeLength * EdgeLength), 0.0f, 1.0f);
									const FVector ClosestPoint = EdgeStart + t * Edge;

									MinDist = FMath::Min(MinDist, FVector::Dist(Point, ClosestPoint));
								}

								return MinDist;
							};

						if (PointToFaceDistance(SphereStart, Face, Normal) <= SphereRadius || PointToFaceDistance(SphereEnd, Face, Normal) <= SphereRadius)
						{
							return true;
						}
					}

					return false; // Simplified for brevity
				};

				if (SphereIntersectsBox(Start, End, Radius, BottomVertices, TopVertices))
				{
					// For box obstacles, use the distance from start to the obstacle's center
					const float DistSqr = FVector::DistSquared(Start, Avoiding.Location);
					ProcessObstacle(Avoiding, DistSqr);
				}
			}
		};

		// 检查静态/动态盒体障碍物
		CheckBoxCollision(Cell.BoxObstacles);
		CheckBoxCollision(Cell.BoxObstaclesStatic);
	}
}


// To Do : 1.Sphere Trace For Subjects(can filter by direction angle)  2.Sphere Sweep For Subjects  3.Sphere Sweep For Subjects Async  4.Sector Trace For Subjects  5.Sector Trace For Subjects Async 
// 
// 1. IgnoreList  2.VisibilityCheck  3.AngleCheck  4.KeepCount  5.SortByDist  6.Async


//--------------------------------------------Avoidance---------------------------------------------------------------

// Performance Test Result : 5.5 ms per 10000 agents on AMD Ryzen 5900X.

void UNeighborGridComponent::Update()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("RVO2 Update");

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("ResetCells");

		ParallelFor(Cells.Num(),[&](int32 Index) // To do : only process occupied cells not all cells
		{
			Cells[Index].Reset();
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
			if (UNLIKELY(!IsInside(Located.Location))) return;

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

			//uint32 ThreadId = FPlatformTLS::GetCurrentThreadId();
			//uint32 index = ThreadId % OccupiedCellsArrays.Num();
			//FOccupiedCells& OccupiedCells = OccupiedCellsArrays[index];

			auto& Cell = At(WorldToCage(Location));

			Cell.Lock();
			Cell.SubjectFingerprint.Add(Subject.GetFingerprint());
			Cell.Subjects.Add(Avoiding);
			Cell.Unlock();

			//OccupiedCells.Lock();
			//OccupiedCells.Cells.Add(GetIndexAt(Location));
			//OccupiedCells.Unlock();

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

			//uint32 ThreadId = FPlatformTLS::GetCurrentThreadId();
			//uint32 index = ThreadId % OccupiedCellsArrays.Num();
			//FOccupiedCells& OccupiedCells = OccupiedCellsArrays[index];

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

						//OccupiedCells.Lock();
						//OccupiedCells.Cells.Add(GetIndexAt(Location));
						//OccupiedCells.Unlock();
					}
				}
			}

		}, ThreadsCount, BatchSize);
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("RegisterSphereObstacles");

		FFilter Filter = FFilter::Make<FLocated, FCollider, FAvoiding, FSphereObstacle>();
		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently([&](FSolidSubjectHandle Subject, FSphereObstacle& SphereObstacle, FLocated& Located, FCollider& Collider, FAvoiding& Avoiding)
		{
			if (SphereObstacle.bStatic && SphereObstacle.bRegistered) return; // if static, we only register once

			const auto Location = Located.Location;

			if (UNLIKELY(!IsInside(Location))) return;

			Avoiding.Location = Location;
			Avoiding.Radius = Collider.Radius;

			//uint32 ThreadId = FPlatformTLS::GetCurrentThreadId();
			//uint32 index = ThreadId % OccupiedCellsArrays.Num();
			//FOccupiedCells& OccupiedCells = OccupiedCellsArrays[index];

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
						if (SphereObstacle.bStatic)
						{
							Cell.SphereObstacleFingerprintStatic.Add(Subject.GetFingerprint());
							Cell.SphereObstaclesStatic.Add(Avoiding);
						}
						else
						{
							Cell.SphereObstacleFingerprint.Add(Subject.GetFingerprint());
							Cell.SphereObstacles.Add(Avoiding);
						}
						Cell.Unlock();

						//OccupiedCells.Lock();
						//OccupiedCells.Cells.Add(GetIndexAt(Location));
						//OccupiedCells.Unlock();
					}
				}
			}

			SphereObstacle.bRegistered = true;

		}, ThreadsCount, BatchSize);
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("RegisterBoxObstacles");

		FFilter Filter = FFilter::Make<FBoxObstacle, FLocated, FAvoiding>();
		auto Chain = Mechanism->EnchainSolid(Filter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, 1, ThreadsCount, BatchSize);

		Chain->OperateConcurrently([&](FSolidSubjectHandle Subject, FBoxObstacle& BoxObstacle, FAvoiding& Avoiding)
		{
			if (BoxObstacle.bStatic && BoxObstacle.bRegistered) return; // if static, we only register once

			const auto Location = BoxObstacle.point3d_;
			Avoiding.Location = Location;

			const FBoxObstacle* PreObstaclePtr = BoxObstacle.prevObstacle_.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();
			const FBoxObstacle* NextObstaclePtr = BoxObstacle.nextObstacle_.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();

			if (NextObstaclePtr == nullptr || PreObstaclePtr == nullptr) return;

			const FVector NextLocation = NextObstaclePtr->point3d_;
			const float ObstacleHeight = BoxObstacle.height_;

			if (IsInside(Location) && IsInside(NextLocation))
			{
				//uint32 ThreadId = FPlatformTLS::GetCurrentThreadId();
				//uint32 index = ThreadId % OccupiedCellsArrays.Num();
				//FOccupiedCells& OccupiedCells = OccupiedCellsArrays[index];

				const float StartZ = Location.Z;
				const float EndZ = StartZ + ObstacleHeight;
				TSet<FIntVector> AllGridCells;
				TArray<FIntVector> AllGridCellsArray;

				float CurrentLayerZ = StartZ;

				while (CurrentLayerZ < EndZ)
				{
					const FVector LayerCurrentPoint = FVector(Location.X, Location.Y, CurrentLayerZ);
					const FVector LayerNextPoint = FVector(NextLocation.X, NextLocation.Y, CurrentLayerZ);
					auto LayerCells = SphereSweepForCells(LayerCurrentPoint, LayerNextPoint, CellSize * 2);

					for (const auto& CellPos : LayerCells)
					{
						AllGridCells.Add(CellPos);
					}

					CurrentLayerZ += CellSize;
				}

				AllGridCellsArray = AllGridCells.Array();

				ParallelFor(AllGridCellsArray.Num(),[&](int32 Index)
				{
					const FIntVector& CellPos = AllGridCellsArray[Index];
					if (!LIKELY(IsInside(CellPos))) return;
					auto& Cell = At(CellPos);

					Cell.Lock();
					if (BoxObstacle.bStatic)
					{
						Cell.BoxObstacleFingerprintStatic.Add(Subject.GetFingerprint());
						Cell.BoxObstaclesStatic.Add(Avoiding);
					}
					else
					{
						Cell.BoxObstacleFingerprint.Add(Subject.GetFingerprint());
						Cell.BoxObstacles.Add(Avoiding);
					}
					Cell.Unlock();

					//OccupiedCells.Lock();
					//OccupiedCells.Cells.Add(GetIndexAt(Location));
					//OccupiedCells.Unlock();
				});
			}

			BoxObstacle.bRegistered = true;

		}, ThreadsCount, BatchSize);
	}
}

void UNeighborGridComponent::Decouple()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("RVO2 Decouple");

	const float DeltaTime = FMath::Clamp(GetWorld()->GetDeltaSeconds(),0,0.03f);

	const FFilter SphereObstacleFilter = FFilter::Make<FSphereObstacle, FAvoiding, FAvoidance, FLocated, FCollider>();
	const FFingerprint SphereObstacleFingerprint = SphereObstacleFilter.GetFingerprint();

	const FFilter BoxObstacleFilter = FFilter::Make<FBoxObstacle, FAvoiding, FLocated>();
	const FFingerprint BoxObstacleFingerprint = BoxObstacleFilter.GetFingerprint();

	AMechanism* Mechanism = GetMechanism();
	const FFilter Filter = FFilter::Make<FAgent, FActivated, FLocated, FCollider, FMove, FMoving, FAvoidance, FAvoiding>().Exclude<FSphereObstacle, FAppearing>();

	auto Chain = Mechanism->EnchainSolid(Filter);
	UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

	Chain->OperateConcurrently([&](FSolidSubjectHandle Subject, FMove& Move, FLocated& Located, FCollider& Collider, FMoving& Moving, FAvoidance& Avoidance, FAvoiding& Avoiding)
	{
		//if (UNLIKELY(!Move.bEnable)) return;

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

		//--------------------------Collect Subject Neighbors--------------------------------

		FFilter SubjectFilter = FFilter::Make<FAgent, FActivated, FLocated, FCollider, FAvoidance, FAvoiding>().Exclude<FSphereObstacle, FCorpse>();

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
		auto SubjectCompare = [&](const FAvoiding& A, const FAvoiding& B) 
		{
			return FVector::DistSquared(SelfLocation, A.Location) > FVector::DistSquared(SelfLocation, B.Location);
		};

		TSet<uint32> SeenHashes;

		for (const FIntVector& Coord : NeighbourCellCoords)
		{
			const auto& Cell = At(Coord);

			// Subject Neighbors
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
		}

		//---------------------------Collect Obstacle Neighbors--------------------------------

		TSet<FAvoiding> ValidSphereObstacleNeighbors;
		ValidSphereObstacleNeighbors.Reserve(MaxNeighbors);

		const float ObstacleRange = Avoidance.RVO_TimeHorizon_Obstacle * Avoidance.MaxSpeed + Avoidance.Radius;
		const FVector ObstacleRange3D(ObstacleRange, ObstacleRange, Avoidance.Radius);
		TArray<FIntVector> ObstacleCellCoords = GetNeighborCells(SelfLocation, ObstacleRange3D);

		for (const FIntVector& Coord : ObstacleCellCoords)
		{
			const auto& Cell = At(Coord);

			// 定义处理 BoxObstacles 的 Lambda 函数
			auto ProcessBoxObstacles = [&](const TArray<FAvoiding>& Obstacles, const FFingerprint& Fingerprint)
			{
				if (Fingerprint.Matches(BoxObstacleFingerprint))
				{
					for (FAvoiding AvoData : Obstacles)
					{
						const FSubjectHandle OtherObstacle = AvoData.SubjectHandle;

						if (UNLIKELY(!OtherObstacle.Matches(BoxObstacleFilter))) continue;

						const auto& ObstacleTrait = OtherObstacle.GetTrait<FBoxObstacle>();
						const FVector ObstaclePoint = ObstacleTrait.point3d_;
						const FBoxObstacle* NextObstaclePtr = ObstacleTrait.nextObstacle_.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();

						if (UNLIKELY(NextObstaclePtr == nullptr)) continue;

						const FVector NextPoint = NextObstaclePtr->point3d_;
						const float ObstacleHalfHeight = ObstacleTrait.height_ * 0.5f;

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
							BoxObstacleNeighbors.Add(AvoData);
						}
					}
				}
			};

			ProcessBoxObstacles(Cell.BoxObstacles, Cell.BoxObstacleFingerprint);
			ProcessBoxObstacles(Cell.BoxObstaclesStatic, Cell.BoxObstacleFingerprintStatic);

			// 定义处理 SphereObstacles 的 Lambda 函数
			auto ProcessSphereObstacles = [&](const TArray<FAvoiding>& Obstacles, const FFingerprint& Fingerprint)
				{
					if (Fingerprint.Matches(SphereObstacleFingerprint))
					{
						for (const FAvoiding& AvoData : Obstacles)
						{
							if (UNLIKELY(!AvoData.SubjectHandle.Matches(SphereObstacleFilter))) continue;

							const float DistSqr = FVector::DistSquared(SelfLocation, AvoData.Location);
							const float RadiusSqr = FMath::Square(AvoData.Radius) + TotalRangeSqr;

							if (UNLIKELY(DistSqr > RadiusSqr)) continue;

							SphereObstacleNeighbors.Add(AvoData);
						}
					}
				};

			ProcessSphereObstacles(Cell.SphereObstacles, Cell.SphereObstacleFingerprint);
			ProcessSphereObstacles(Cell.SphereObstaclesStatic, Cell.SphereObstacleFingerprintStatic);
		}

		Located.preLocation = Located.Location;

		Avoidance.Radius = SelfRadius;
		Avoidance.Position = RVO::Vector2(SelfLocation.X, SelfLocation.Y);

		//----------------------------Try Avoid SubjectNeighbors---------------------------------

		Avoidance.MaxSpeed = FMath::Clamp(Move.MoveSpeed * Moving.PassiveSpeedMult, Avoidance.RVO_MinAvoidSpeed, FLT_MAX);
		Avoidance.DesiredVelocity = RVO::Vector2(Moving.DesiredVelocity.X, Moving.DesiredVelocity.Y);//copy into rvo trait
		Avoidance.CurrentVelocity = RVO::Vector2(Moving.CurrentVelocity.X, Moving.CurrentVelocity.Y);//copy into rvo trait
		
		//float PreVelocity2D = Moving.CurrentVelocity.Size2D();
		TArray<FAvoiding> EmptyArray;

		ComputeNewVelocity(Avoidance, SubjectNeighbors, EmptyArray, DeltaTime);

		if (!Moving.bFalling && !(Moving.LaunchTimer > 0))
		{
			FVector AvoidingVelocity(Avoidance.AvoidingVelocity.x(), Avoidance.AvoidingVelocity.y(), 0);
			FVector CurrentVelocity(Avoidance.CurrentVelocity.x(), Avoidance.CurrentVelocity.y(), 0);
			FVector InterpedVelocity = FMath::VInterpConstantTo(CurrentVelocity, AvoidingVelocity, DeltaTime, Move.Acceleration);
			Moving.CurrentVelocity = FVector(InterpedVelocity.X, InterpedVelocity.Y, Moving.CurrentVelocity.Z); // velocity can only change so much because of inertia
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
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("ComputeNewVelocity");

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


//---------------------------------------------Helpers------------------------------------------------------------------

TArray<FIntVector> UNeighborGridComponent::GetNeighborCells(const FVector& Center, const FVector& Range3D) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("GetNeighborCells");

	const FIntVector Min = WorldToCage(Center - Range3D);
	const FIntVector Max = WorldToCage(Center + Range3D);

	TArray<FIntVector> ValidCells;
	const int32 ExpectedCells = (Max.X - Min.X + 1) * (Max.Y - Min.Y + 1) * (Max.Z - Min.Z + 1);
	ValidCells.Reserve(ExpectedCells); // 根据场景规模调整

	for (int32 z = Min.Z; z <= Max.Z; ++z) {
		for (int32 y = Min.Y; y <= Max.Y; ++y) {
			for (int32 x = Min.X; x <= Max.X; ++x) {
				const FIntVector CellPos(x, y, z);
				if (LIKELY(IsInside(CellPos))) { // 分支预测优化
					ValidCells.Emplace(CellPos);
				}
			}
		}
	}
	return ValidCells;
}

TArray<FIntVector> UNeighborGridComponent::SphereSweepForCells(FVector Start, FVector End, float Radius) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("SphereSweepForCells");

	// 预计算关键参数
	const float RadiusInCellsValue = Radius / CellSize;
	const int32 RadiusInCells = FMath::CeilToInt(RadiusInCellsValue);
	const float RadiusSq = FMath::Square(RadiusInCellsValue);

	// 改用TArray+BitArray加速去重
	TArray<FIntVector> GridCells;
	TSet<FIntVector> GridCellsSet; // 保持TSet或根据性能测试调整

	FIntVector StartCell = WorldToCage(Start);
	FIntVector EndCell = WorldToCage(End);
	FIntVector Delta = EndCell - StartCell;
	FIntVector AbsDelta(FMath::Abs(Delta.X), FMath::Abs(Delta.Y), FMath::Abs(Delta.Z));

	// Bresenham算法参数
	FIntVector CurrentCell = StartCell;
	const int32 XStep = Delta.X > 0 ? 1 : -1;
	const int32 YStep = Delta.Y > 0 ? 1 : -1;
	const int32 ZStep = Delta.Z > 0 ? 1 : -1;

	// 主循环
	if (AbsDelta.X >= AbsDelta.Y && AbsDelta.X >= AbsDelta.Z)
	{
		// X为主轴
		int32 P1 = 2 * AbsDelta.Y - AbsDelta.X;
		int32 P2 = 2 * AbsDelta.Z - AbsDelta.X;

		for (int32 i = 0; i < AbsDelta.X; ++i)
		{
			AddSphereCells(CurrentCell, RadiusInCells, RadiusSq, GridCellsSet);
			if (P1 >= 0) { CurrentCell.Y += YStep; P1 -= 2 * AbsDelta.X; }
			if (P2 >= 0) { CurrentCell.Z += ZStep; P2 -= 2 * AbsDelta.X; }
			P1 += 2 * AbsDelta.Y;
			P2 += 2 * AbsDelta.Z;
			CurrentCell.X += XStep;
		}
	}
	else if (AbsDelta.Y >= AbsDelta.Z)
	{
		// Y为主轴
		int32 P1 = 2 * AbsDelta.X - AbsDelta.Y;
		int32 P2 = 2 * AbsDelta.Z - AbsDelta.Y;
		for (int32 i = 0; i < AbsDelta.Y; ++i)
		{
			AddSphereCells(CurrentCell, RadiusInCells, RadiusSq, GridCellsSet);
			if (P1 >= 0) { CurrentCell.X += XStep; P1 -= 2 * AbsDelta.Y; }
			if (P2 >= 0) { CurrentCell.Z += ZStep; P2 -= 2 * AbsDelta.Y; }
			P1 += 2 * AbsDelta.X;
			P2 += 2 * AbsDelta.Z;
			CurrentCell.Y += YStep;
		}
	}
	else
	{
		// Z为主轴
		int32 P1 = 2 * AbsDelta.X - AbsDelta.Z;
		int32 P2 = 2 * AbsDelta.Y - AbsDelta.Z;
		for (int32 i = 0; i < AbsDelta.Z; ++i)
		{
			AddSphereCells(CurrentCell, RadiusInCells, RadiusSq, GridCellsSet);
			if (P1 >= 0) { CurrentCell.X += XStep; P1 -= 2 * AbsDelta.Z; }
			if (P2 >= 0) { CurrentCell.Y += YStep; P2 -= 2 * AbsDelta.Z; }
			P1 += 2 * AbsDelta.X;
			P2 += 2 * AbsDelta.Y;
			CurrentCell.Z += ZStep;
		}
	}

	AddSphereCells(EndCell, RadiusInCells, RadiusSq, GridCellsSet);

	// 转换为数组并排序
	TArray<FIntVector> ResultArray = GridCellsSet.Array();

	// 按距离Start点的平方距离排序（避免开根号）
	Algo::Sort(ResultArray, [this, Start](const FIntVector& A, const FIntVector& B)
		{
			const FVector WorldA = CageToWorld(A);
			const FVector WorldB = CageToWorld(B);
			return (WorldA - Start).SizeSquared() < (WorldB - Start).SizeSquared();
		});

	return ResultArray;
}

void UNeighborGridComponent::AddSphereCells(FIntVector CenterCell, int32 RadiusInCells, float RadiusSq, TSet<FIntVector>& GridCells) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("AddSphereCells");

	// 分层遍历优化：减少无效循环
	for (int32 x = -RadiusInCells; x <= RadiusInCells; ++x)
	{
		const int32 XSq = x * x;
		if (XSq > RadiusSq) continue;

		for (int32 y = -RadiusInCells; y <= RadiusInCells; ++y)
		{
			const int32 YSq = y * y;
			if (XSq + YSq > RadiusSq) continue;

			// 直接计算z范围，减少循环次数
			const float RemainingSq = RadiusSq - (XSq + YSq);
			const int32 MaxZ = FMath::FloorToInt(FMath::Sqrt(RemainingSq));
			for (int32 z = -MaxZ; z <= MaxZ; ++z)
			{
				GridCells.Add(CenterCell + FIntVector(x, y, z));
			}
		}
	}
}
