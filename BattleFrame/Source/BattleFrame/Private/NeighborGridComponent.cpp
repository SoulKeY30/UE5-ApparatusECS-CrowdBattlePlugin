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
#include "BitMask.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "BattleFrameFunctionLibraryRT.h"
#include "Kismet/BlueprintAsyncActionBase.h"

UNeighborGridComponent::UNeighborGridComponent()
{
	bWantsInitializeComponent = true;
}

void UNeighborGridComponent::BeginPlay()
{
	Super::BeginPlay();

	DefineFilters();
}

void UNeighborGridComponent::InitializeComponent()
{
	DoInitializeCells();
	GetBounds();
}

void UNeighborGridComponent::DefineFilters()
{
	RegisterNeighborGrid_Trace_Filter = FFilter::Make<FLocated, FTrace, FActivated>();
	RegisterNeighborGrid_SphereObstacle_Filter = FFilter::Make<FLocated, FSphereObstacle>();
	RegisterSubjectFilter = FFilter::Make<FLocated, FCollider, FTrace, FGridData, FActivated>();
	RegisterSubjectSingleFilter = FFilter::Make<FLocated, FCollider, FGridData, FActivated>().Exclude<FRegisterMultiple>();
	RegisterSubjectMultipleFilter = FFilter::Make<FLocated, FCollider, FGridData, FRegisterMultiple, FActivated>().Exclude<FSphereObstacle>();
	RegisterSphereObstaclesFilter = FFilter::Make<FLocated, FCollider, FGridData, FSphereObstacle>();
	RegisterBoxObstaclesFilter = FFilter::Make<FBoxObstacle, FLocated, FGridData>();
	SubjectFilterBase = FFilter::Make<FLocated, FCollider, FAvoidance, FAvoiding, FGridData, FActivated>().Exclude<FSphereObstacle, FBoxObstacle, FCorpse>();
	SphereObstacleFilter = FFilter::Make<FSphereObstacle, FGridData, FAvoidance, FAvoiding, FLocated, FCollider>();
	BoxObstacleFilter = FFilter::Make<FBoxObstacle, FGridData, FLocated>();
	DecoupleFilter = FFilter::Make<FAgent, FLocated, FDirected, FCollider, FMove, FMoving, FAvoidance, FAvoiding, FGridData, FActivated>().Exclude<FAppearing>();
}


//--------------------------------------------Tracing----------------------------------------------------------------

// Multi Trace For Subjects
void UNeighborGridComponent::SphereTraceForSubjects
(
	int32 KeepCount,
	const FVector& Origin,
	const float Radius,
	const bool bCheckVisibility,
	const FVector& CheckOrigin,
	const float CheckRadius,
	ESortMode SortMode,
	const FVector& SortOrigin,
	const FSubjectArray& IgnoreSubjects,
	const FFilter& Filter,
	bool& Hit,
	TArray<FTraceResult>& Results
) const
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("SphereTraceForSubjects");
	Results.Reset();

	// 特殊处理标志
	const bool bSingleResult = (KeepCount == 1);
	const bool bNoCountLimit = (KeepCount == -1);

	// 将忽略列表转换为集合以便快速查找
	const TSet<FSubjectHandle> IgnoreSet(IgnoreSubjects.Subjects);

	// 扩展搜索范围 - 使用各轴独立的CellSize
	const FVector CellRadius = CellSize * 0.5f;
	const float MaxCellRadius = FMath::Max3(CellRadius.X, CellRadius.Y, CellRadius.Z);
	const float ExpandedRadius = Radius + MaxCellRadius * FMath::Sqrt(2.0f);
	const FVector Range(ExpandedRadius);

	const FIntVector CagePosMin = WorldToCage(Origin - Range);
	const FIntVector CagePosMax = WorldToCage(Origin + Range);

	// 跟踪最佳结果（用于KeepCount=1的情况）
	FTraceResult BestResult;
	float BestDistSq = (SortMode == ESortMode::NearToFar) ? FLT_MAX : -FLT_MAX;

	// 临时存储所有结果（用于需要排序或随机的情况）
	TArray<FTraceResult> TempResults;

	// 预收集候选格子并按距离排序
	TArray<FIntVector> CandidateCells;

	for (int32 z = CagePosMin.Z; z <= CagePosMax.Z; ++z)
	{
		for (int32 y = CagePosMin.Y; y <= CagePosMax.Y; ++y)
		{
			for (int32 x = CagePosMin.X; x <= CagePosMax.X; ++x)
			{
				const FIntVector CellPos(x, y, z);
				if (!IsInside(CellPos)) continue;

				const FVector CellCenter = CageToWorld(CellPos);
				const float DistSq = FVector::DistSquared(CellCenter, Origin);

				if (DistSq > FMath::Square(ExpandedRadius)) continue;

				CandidateCells.Add(CellPos);
			}
		}
	}

	// 根据SortMode对格子进行排序
	if (SortMode != ESortMode::None)
	{
		CandidateCells.Sort([this, SortOrigin, SortMode](const FIntVector& A, const FIntVector& B) {
			const float DistSqA = FVector::DistSquared(CageToWorld(A), SortOrigin);
			const float DistSqB = FVector::DistSquared(CageToWorld(B), SortOrigin);
			return SortMode == ESortMode::NearToFar ? DistSqA < DistSqB : DistSqA > DistSqB;
			});
	}

	// 计算提前终止阈值
	float ThresholdDistanceSq = FLT_MAX;
	if (!bNoCountLimit && KeepCount > 0 && SortMode != ESortMode::None)
	{
		// 计算阈值：当前最远结果的距离 + 2倍格子对角线距离（使用最大轴尺寸）
		const float MaxCellSize = CellSize.GetMax();
		const float ThresholdDistance = FMath::Sqrt(BestDistSq) + 2.0f * MaxCellSize * FMath::Sqrt(2.0f);
		ThresholdDistanceSq = FMath::Square(ThresholdDistance);
	}

	// 遍历检测
	for (const FIntVector& CellPos : CandidateCells)
	{
		// 提前终止检查
		if (!bNoCountLimit && KeepCount > 0 && SortMode != ESortMode::None)
		{
			const float CellDistSq = FVector::DistSquared(CageToWorld(CellPos), SortOrigin);
			if ((SortMode == ESortMode::NearToFar && CellDistSq > ThresholdDistanceSq) ||
				(SortMode == ESortMode::FarToNear && CellDistSq < ThresholdDistanceSq))
			{
				break;
			}
		}

		const auto& CellData = At(CellPos);

		for (const FGridData& SubjectData : CellData.Subjects)
		{
			const FSubjectHandle Subject = SubjectData.SubjectHandle;
			if (IgnoreSet.Contains(Subject)) continue;
			if (!Subject.Matches(Filter)) continue;

			const FVector SubjectPos = FVector(SubjectData.Location);
			const float SubjectRadius = SubjectData.Radius;

			// 距离检查
			const FVector Delta = SubjectPos - Origin;
			const float DistSq = Delta.SizeSquared();
			const float CombinedRadius = Radius + SubjectRadius;
			if (DistSq > FMath::Square(CombinedRadius)) continue;

			if (bCheckVisibility)
			{
				bool bVisibilityHit = false;
				FTraceResult VisibilityResult;

				const FVector ToSubjectDir = (SubjectPos - CheckOrigin).GetSafeNormal();
				const FVector SubjectSurfacePoint = SubjectPos - (ToSubjectDir * SubjectRadius);

				SphereSweepForObstacle(CheckOrigin, SubjectSurfacePoint, CheckRadius, bVisibilityHit, VisibilityResult);

				if (bVisibilityHit) continue;
			}

			const float CurrentDistSq = FVector::DistSquared(SortOrigin, SubjectPos);

			if (bSingleResult)
			{
				bool bIsBetter = false;
				if (SortMode == ESortMode::NearToFar)
				{
					bIsBetter = (CurrentDistSq < BestDistSq);
				}
				else if (SortMode == ESortMode::FarToNear)
				{
					bIsBetter = (CurrentDistSq > BestDistSq);
				}
				else // ESortMode::None
				{
					bIsBetter = true;
				}

				if (bIsBetter)
				{
					BestResult.Subject = Subject;
					BestResult.Location = SubjectPos;
					BestResult.CachedDistSq = CurrentDistSq;
					BestDistSq = CurrentDistSq;

					// 更新阈值
					if (!bNoCountLimit && KeepCount > 0)
					{
						const float MaxCellSize = CellSize.GetMax();
						const float ThresholdDistance = FMath::Sqrt(BestDistSq) + 2.0f * MaxCellSize * FMath::Sqrt(2.0f);
						ThresholdDistanceSq = FMath::Square(ThresholdDistance);
					}
				}
			}
			else
			{
				FTraceResult Result;
				Result.Subject = Subject;
				Result.Location = SubjectPos;
				Result.CachedDistSq = CurrentDistSq;
				TempResults.Add(Result);

				// 当收集到足够结果时更新阈值
				if (!bNoCountLimit && KeepCount > 0 && SortMode != ESortMode::None && TempResults.Num() >= KeepCount)
				{
					// 找到当前第KeepCount个最佳结果的距离
					float CurrentThresholdDistSq = 0.0f;
					if (SortMode == ESortMode::NearToFar)
					{
						CurrentThresholdDistSq = TempResults[KeepCount - 1].CachedDistSq;
					}
					else // FarToNear
					{
						CurrentThresholdDistSq = TempResults.Last().CachedDistSq;
					}

					// 更新阈值
					const float MaxCellSize = CellSize.GetMax();
					const float ThresholdDistance = FMath::Sqrt(CurrentThresholdDistSq) + 2.0f * MaxCellSize * FMath::Sqrt(2.0f);
					ThresholdDistanceSq = FMath::Square(ThresholdDistance);
				}
			}
		}
	}

	// 处理结果
	if (bSingleResult)
	{
		if (BestDistSq != FLT_MAX && BestDistSq != -FLT_MAX)
		{
			Results.Add(BestResult);
		}
	}
	else if (TempResults.Num() > 0)
	{
		// 处理排序或随机化
		if (SortMode != ESortMode::None)
		{
			// 应用排序（无论KeepCount是否为-1）
			TempResults.Sort([SortMode](const FTraceResult& A, const FTraceResult& B) {
				return SortMode == ESortMode::NearToFar ?
					A.CachedDistSq < B.CachedDistSq :
					A.CachedDistSq > B.CachedDistSq;
				});
		}
		else
		{
			// 随机打乱结果
			if (TempResults.Num() > 1)
			{
				for (int32 i = TempResults.Num() - 1; i > 0; --i)
				{
					const int32 j = FMath::Rand() % (i + 1);
					TempResults.Swap(i, j);
				}
			}
		}

		// 应用数量限制（只有当KeepCount>0时）
		if (!bNoCountLimit && KeepCount > 0 && TempResults.Num() > KeepCount)
		{
			TempResults.SetNum(KeepCount);
		}

		Results.Append(TempResults);
	}

	Hit = !Results.IsEmpty();
}

// Multi Sweep Trace For Subjects
void UNeighborGridComponent::SphereSweepForSubjects
(
	int32 KeepCount,
	const FVector& Start,
	const FVector& End,
	const float Radius,
	const bool bCheckVisibility,
	const FVector& CheckOrigin,
	const float CheckRadius,
	ESortMode SortMode,
	const FVector& SortOrigin,
	const FSubjectArray& IgnoreSubjects,
	const FFilter& Filter,
	bool& Hit,
	TArray<FTraceResult>& Results
) const
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("SphereSweepForSubjects");
	Results.Reset(); // Clear result array

	// Convert ignore list to set for fast lookup
	const TSet<FSubjectHandle> IgnoreSet(IgnoreSubjects.Subjects);

	// Get cells along the sweep path (already handles FVector CellSize)
	TArray<FIntVector> GridCells = SphereSweepForCells(Start, End, Radius);

	const FVector TraceDir = (End - Start).GetSafeNormal();
	const float TraceLength = FVector::Distance(Start, End);

	// Temporary array to store unsorted results
	TArray<FTraceResult> TempResults;

	// Check subjects in each cell
	for (const FIntVector& CellIndex : GridCells)
	{
		if (!IsInside(CellIndex)) continue;

		const auto& CageCell = At(CellIndex.X, CellIndex.Y, CellIndex.Z);

		for (const FGridData& Data : CageCell.Subjects)
		{
			const FSubjectHandle Subject = Data.SubjectHandle;

			// Check if in ignore list
			if (IgnoreSet.Contains(Subject)) continue;

			// Validity checks
			if (!Subject.IsValid() || !Subject.Matches(Filter)) continue;

			const FVector SubjectPos = FVector(Data.Location);
			float SubjectRadius = Data.Radius;

			// Distance calculations
			const FVector ToSubject = SubjectPos - Start;
			const float ProjOnTrace = FVector::DotProduct(ToSubject, TraceDir);

			// Initial filtering
			const float ProjThreshold = SubjectRadius + Radius;
			if (ProjOnTrace < -ProjThreshold || ProjOnTrace > TraceLength + ProjThreshold) continue;

			// Precise distance check (still spherical)
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

				// Create FTraceResult and add to temp results array
				FTraceResult Result;
				Result.Subject = Subject;
				Result.Location = SubjectPos;
				Result.CachedDistSq = FVector::DistSquared(SortOrigin, SubjectPos);
				TempResults.Add(Result);
			}
		}
	}

	// Sorting logic
	if (SortMode != ESortMode::None)
	{
		TempResults.Sort([SortMode](const FTraceResult& A, const FTraceResult& B)
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

	// Apply KeepCount limit
	const bool bHasLimit = (KeepCount > 0);
	int32 Count = 0;

	for (const FTraceResult& Result : TempResults)
	{
		Results.Add(Result);
		Count++;

		if (bHasLimit && Count >= KeepCount)
		{
			break;
		}
	}

	Hit = !Results.IsEmpty();
}

void UNeighborGridComponent::SectorTraceForSubjects
(
	int32 KeepCount,
	const FVector& Origin,
	const float Radius,
	const float Height,
	const FVector& Direction,
	const float Angle,
	const bool bCheckVisibility,
	const FVector& CheckOrigin,
	const float CheckRadius,
	ESortMode SortMode,
	const FVector& SortOrigin,
	const FSubjectArray& IgnoreSubjects,
	const FFilter& Filter,
	bool& Hit,
	TArray<FTraceResult>& Results
) const
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("SectorTraceForSubjects");
	Results.Reset();

	// 特殊处理标志
	const bool bFullCircle = FMath::IsNearlyEqual(Angle, 360.0f, KINDA_SMALL_NUMBER);
	const bool bSingleResult = (KeepCount == 1);
	const bool bNoCountLimit = (KeepCount == -1);

	// 将忽略列表转换为集合以便快速查找
	const TSet<FSubjectHandle> IgnoreSet(IgnoreSubjects.Subjects);

	const FVector NormalizedDir = Direction.GetSafeNormal2D();
	const float HalfAngleRad = FMath::DegreesToRadians(Angle * 0.5f);
	const float CosHalfAngle = FMath::Cos(HalfAngleRad);

	// 计算扇形的两个边界方向（如果不是全圆）
	FVector LeftBoundDir, RightBoundDir;
	if (!bFullCircle)
	{
		LeftBoundDir = NormalizedDir.RotateAngleAxis(Angle * 0.5f, FVector::UpVector);
		RightBoundDir = NormalizedDir.RotateAngleAxis(-Angle * 0.5f, FVector::UpVector);
	}

	// 扩展搜索范围 - 使用各轴独立的CellSize
	const FVector CellRadius = CellSize * 0.5f;
	const float ExpandedRadiusXY = Radius + FMath::Max(CellRadius.X, CellRadius.Y) * FMath::Sqrt(2.0f);
	const float ExpandedHeight = Height / 2.0f + CellRadius.Z;
	const FVector Range(ExpandedRadiusXY, ExpandedRadiusXY, ExpandedHeight);

	const FIntVector CagePosMin = WorldToCage(Origin - Range);
	const FIntVector CagePosMax = WorldToCage(Origin + Range);

	// 跟踪最佳结果（用于KeepCount=1的情况）
	FTraceResult BestResult;
	float BestDistSq = (SortMode == ESortMode::NearToFar) ? FLT_MAX : -FLT_MAX;

	// 临时存储所有结果（用于需要排序或随机的情况）
	TArray<FTraceResult> TempResults;

	// 预收集候选格子并按距离排序
	TArray<FIntVector> CandidateCells;

	for (int32 z = CagePosMin.Z; z <= CagePosMax.Z; ++z)
	{
		for (int32 x = CagePosMin.X; x <= CagePosMax.X; ++x)
		{
			for (int32 y = CagePosMin.Y; y <= CagePosMax.Y; ++y)
			{
				const FIntVector CellPos(x, y, z);
				if (!IsInside(CellPos)) continue;

				const FVector CellCenter = CageToWorld(CellPos);
				const FVector DeltaXY = (CellCenter - Origin) * FVector(1, 1, 0);
				const float DistSqXY = DeltaXY.SizeSquared();

				if (DistSqXY > FMath::Square(ExpandedRadiusXY)) continue;

				const float VerticalDist = FMath::Abs(CellCenter.Z - Origin.Z);
				if (VerticalDist > ExpandedHeight) continue;

				if (!bFullCircle && DistSqXY > SMALL_NUMBER)
				{
					const FVector ToCellDirXY = DeltaXY.GetSafeNormal();
					const float DotProduct = FVector::DotProduct(NormalizedDir, ToCellDirXY);

					if (DotProduct < CosHalfAngle)
					{
						FVector ClosestPoint;
						float DistToLeftBound = FMath::PointDistToLine(CellCenter, Origin, LeftBoundDir, ClosestPoint);
						if (DistToLeftBound >= FMath::Max(CellRadius.X, CellRadius.Y))
						{
							float DistToRightBound = FMath::PointDistToLine(CellCenter, Origin, RightBoundDir, ClosestPoint);
							if (DistToRightBound >= FMath::Max(CellRadius.X, CellRadius.Y))
							{
								const FVector CellMin = CageToWorld(CellPos) - CellRadius;
								const FVector CellMax = CageToWorld(CellPos) + CellRadius;
								if (!(CellMin.X <= Origin.X && Origin.X <= CellMax.X &&
									CellMin.Y <= Origin.Y && Origin.Y <= CellMax.Y))
								{
									continue;
								}
							}
						}
					}
				}
				CandidateCells.Add(CellPos);
			}
		}
	}

	// 根据SortMode对格子进行排序
	if (SortMode != ESortMode::None)
	{
		CandidateCells.Sort([this, SortOrigin, SortMode](const FIntVector& A, const FIntVector& B) {
			const float DistSqA = FVector::DistSquared(CageToWorld(A), SortOrigin);
			const float DistSqB = FVector::DistSquared(CageToWorld(B), SortOrigin);
			return SortMode == ESortMode::NearToFar ? DistSqA < DistSqB : DistSqA > DistSqB;
			});
	}

	// 计算提前终止阈值
	float ThresholdDistanceSq = FLT_MAX;
	if (!bNoCountLimit && KeepCount > 0 && SortMode != ESortMode::None)
	{
		// 计算阈值：当前最远结果的距离 + 2倍格子对角线距离（使用最大轴尺寸）
		const float MaxCellSize = CellSize.GetMax();
		const float ThresholdDistance = FMath::Sqrt(BestDistSq) + 2.0f * MaxCellSize * FMath::Sqrt(2.0f);
		ThresholdDistanceSq = FMath::Square(ThresholdDistance);
	}

	// 遍历检测
	for (const FIntVector& CellPos : CandidateCells)
	{
		// 提前终止检查
		if (!bNoCountLimit && KeepCount > 0 && SortMode != ESortMode::None)
		{
			const float CellDistSq = FVector::DistSquared(CageToWorld(CellPos), SortOrigin);
			if ((SortMode == ESortMode::NearToFar && CellDistSq > ThresholdDistanceSq) ||
				(SortMode == ESortMode::FarToNear && CellDistSq < ThresholdDistanceSq))
			{
				break;
			}
		}

		const auto& CellData = At(CellPos);

		for (const FGridData& SubjectData : CellData.Subjects)
		{
			const FSubjectHandle Subject = SubjectData.SubjectHandle;
			if (IgnoreSet.Contains(Subject)) continue;
			if (!Subject.Matches(Filter)) continue;

			const FVector SubjectPos = FVector(SubjectData.Location);
			const float SubjectRadius = SubjectData.Radius;

			// 高度检查
			const float VerticalDist = FMath::Abs(SubjectPos.Z - Origin.Z);
			if (VerticalDist > (Height / 2.0f + SubjectRadius)) continue;

			// 距离检查
			const FVector DeltaXY = (SubjectPos - Origin) * FVector(1, 1, 0);
			const float DistSqXY = DeltaXY.SizeSquared();
			const float CombinedRadius = Radius + SubjectRadius;
			if (DistSqXY > FMath::Square(CombinedRadius)) continue;

			// 角度检查
			if (!bFullCircle && DistSqXY > SMALL_NUMBER)
			{
				const FVector ToSubjectDirXY = DeltaXY.GetSafeNormal();
				const float DotProduct = FVector::DotProduct(NormalizedDir, ToSubjectDirXY);
				if (DotProduct < CosHalfAngle) continue;
			}

			if (bCheckVisibility)
			{
				bool bVisibilityHit = false;
				FTraceResult VisibilityResult;

				const FVector ToSubjectDir = (SubjectPos - CheckOrigin).GetSafeNormal();
				const FVector SubjectSurfacePoint = SubjectPos - (ToSubjectDir * SubjectRadius);

				SphereSweepForObstacle(CheckOrigin, SubjectSurfacePoint, CheckRadius, bVisibilityHit, VisibilityResult);

				if (bVisibilityHit) continue;
			}

			const float CurrentDistSq = FVector::DistSquared(SortOrigin, SubjectPos);

			if (bSingleResult)
			{
				bool bIsBetter = false;
				if (SortMode == ESortMode::NearToFar)
				{
					bIsBetter = (CurrentDistSq < BestDistSq);
				}
				else if (SortMode == ESortMode::FarToNear)
				{
					bIsBetter = (CurrentDistSq > BestDistSq);
				}
				else // ESortMode::None
				{
					bIsBetter = true;
				}

				if (bIsBetter)
				{
					BestResult.Subject = Subject;
					BestResult.Location = SubjectPos;
					BestResult.CachedDistSq = CurrentDistSq;
					BestDistSq = CurrentDistSq;

					// 更新阈值
					if (!bNoCountLimit && KeepCount > 0)
					{
						const float MaxCellSize = CellSize.GetMax();
						const float ThresholdDistance = FMath::Sqrt(BestDistSq) + 2.0f * MaxCellSize * FMath::Sqrt(2.0f);
						ThresholdDistanceSq = FMath::Square(ThresholdDistance);
					}
				}
			}
			else
			{
				FTraceResult Result;
				Result.Subject = Subject;
				Result.Location = SubjectPos;
				Result.CachedDistSq = CurrentDistSq;
				TempResults.Add(Result);

				// 当收集到足够结果时更新阈值
				if (!bNoCountLimit && KeepCount > 0 && SortMode != ESortMode::None && TempResults.Num() >= KeepCount)
				{
					// 找到当前第KeepCount个最佳结果的距离
					float CurrentThresholdDistSq = 0.0f;
					if (SortMode == ESortMode::NearToFar)
					{
						CurrentThresholdDistSq = TempResults[KeepCount - 1].CachedDistSq;
					}
					else // FarToNear
					{
						CurrentThresholdDistSq = TempResults.Last().CachedDistSq;
					}

					// 更新阈值
					const float MaxCellSize = CellSize.GetMax();
					const float ThresholdDistance = FMath::Sqrt(CurrentThresholdDistSq) + 2.0f * MaxCellSize * FMath::Sqrt(2.0f);
					ThresholdDistanceSq = FMath::Square(ThresholdDistance);
				}
			}
		}
	}

	// 处理结果
	if (bSingleResult)
	{
		if (BestDistSq != FLT_MAX && BestDistSq != -FLT_MAX)
		{
			Results.Add(BestResult);
		}
	}
	else if (TempResults.Num() > 0)
	{
		// 处理排序或随机化
		if (SortMode != ESortMode::None)
		{
			// 应用排序（无论KeepCount是否为-1）
			TempResults.Sort([SortMode](const FTraceResult& A, const FTraceResult& B) {
				return SortMode == ESortMode::NearToFar ?
					A.CachedDistSq < B.CachedDistSq :
					A.CachedDistSq > B.CachedDistSq;
				});
		}
		else
		{
			// 随机打乱结果
			if (TempResults.Num() > 1)
			{
				for (int32 i = TempResults.Num() - 1; i > 0; --i)
				{
					const int32 j = FMath::Rand() % (i + 1);
					TempResults.Swap(i, j);
				}
			}
		}

		// 应用数量限制（只有当KeepCount>0时）
		if (!bNoCountLimit && KeepCount > 0 && TempResults.Num() > KeepCount)
		{
			TempResults.SetNum(KeepCount);
		}

		Results.Append(TempResults);
	}

	Hit = !Results.IsEmpty();
}

// Single Sweep Trace For Nearest Obstacle
void UNeighborGridComponent::SphereSweepForObstacle
(
	const FVector& Start,
	const FVector& End,
	float Radius,
	bool& Hit,
	FTraceResult& Result
) const
{
	Hit = false;
	Result = FTraceResult();
	float ClosestHitDistSq = FLT_MAX;

	TArray<FIntVector> PathCells = SphereSweepForCells(Start, End, Radius);
	const FVector CellExtent = CellSize * 0.5f;
	const float CellMaxRadius = CellExtent.GetMax(); // 最长半轴作为单元包围球半径

	auto ProcessObstacle = [&](const FGridData& Obstacle, float DistSqr)
		{
			if (DistSqr < ClosestHitDistSq)
			{
				ClosestHitDistSq = DistSqr;
				Hit = true;
				Result.Subject = Obstacle.SubjectHandle;
				Result.Location = FVector(Obstacle.Location);
				Result.CachedDistSq = DistSqr;
			}
		};

	for (const FIntVector& CellPos : PathCells)
	{
		const auto& Cell = At(CellPos);

		// 检查球形障碍物
		auto CheckSphereCollision = [&](const TArray<FGridData, TInlineAllocator<8>>& Obstacles)
			{
				for (const FGridData& GridData : Obstacles)
				{
					if (!GridData.SubjectHandle.IsValid()) continue;

					const FSphereObstacle* CurrentObstacle = GridData.SubjectHandle.GetTraitPtr<FSphereObstacle, EParadigm::Unsafe>();
					if (!CurrentObstacle) continue;
					if (CurrentObstacle->bExcluded) continue;

					// 计算球体到线段的最短距离平方
					const float DistSqr = FMath::PointDistToSegmentSquared(FVector(GridData.Location), Start, End);
					const float CombinedRadius = Radius + GridData.Radius;

					// 如果距离小于合并半径，则发生碰撞
					if (DistSqr <= FMath::Square(CombinedRadius))
					{
						ProcessObstacle(GridData, DistSqr);
					}
				}
			};

		// 检查静态/动态球形障碍物
		CheckSphereCollision(Cell.SphereObstacles);
		CheckSphereCollision(Cell.SphereObstaclesStatic);

		// 检查长方体障碍物碰撞
		auto CheckBoxCollision = [&](const TArray<FGridData, TInlineAllocator<8>>& Obstacles)
			{
				// 预计算常用向量
				const FVector Up = FVector::UpVector;
				const FVector SphereDir = (End - Start).GetSafeNormal();
				const float SphereRadiusSq = Radius * Radius;

				for (const FGridData& GridData : Obstacles)
				{
					if (!GridData.SubjectHandle.IsValid()) continue;

					const FBoxObstacle* CurrentObstacle = GridData.SubjectHandle.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();
					if (!CurrentObstacle || CurrentObstacle->bExcluded) continue;

					// 检查并获取前一个障碍物
					const FBoxObstacle* PrevObstacle = CurrentObstacle->prevObstacle_.IsValid() ?
						CurrentObstacle->prevObstacle_.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>() : nullptr;
					if (!PrevObstacle || PrevObstacle->bExcluded) continue;

					// 检查并获取下一个障碍物
					const FBoxObstacle* NextObstacle = CurrentObstacle->nextObstacle_.IsValid() ?
						CurrentObstacle->nextObstacle_.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>() : nullptr;
					if (!NextObstacle || NextObstacle->bExcluded) continue;

					// 检查并获取下下个障碍物
					const FBoxObstacle* NextNextObstacle = NextObstacle->nextObstacle_.IsValid() ?
						NextObstacle->nextObstacle_.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>() : nullptr;
					if (!NextNextObstacle || NextNextObstacle->bExcluded) continue;

					// 获取所有位置并计算高度相关向量
					const FVector& CurrentPos = CurrentObstacle->point3d_;
					const FVector& PrevPos = PrevObstacle->point3d_;
					const FVector& NextPos = NextObstacle->point3d_;
					const FVector& NextNextPos = NextNextObstacle->point3d_;

					const float HalfHeight = CurrentObstacle->height_ * 0.5f;
					const FVector HalfHeightVec = Up * HalfHeight;

					// 预计算所有顶点
					const FVector BottomVertices[4] =
					{
						PrevPos - HalfHeightVec,
						CurrentPos - HalfHeightVec,
						NextPos - HalfHeightVec,
						NextNextPos - HalfHeightVec
					};

					const FVector TopVertices[4] =
					{
						PrevPos + HalfHeightVec,
						CurrentPos + HalfHeightVec,
						NextPos + HalfHeightVec,
						NextNextPos + HalfHeightVec
					};

					// 优化后的球体与长方体相交检测
					auto SphereIntersectsBox = [&](const FVector& SphereStart, const FVector& SphereEnd, float SphereRadius,
						const FVector BottomVerts[4], const FVector TopVerts[4]) -> bool
						{
							// 1. 快速AABB测试
							FVector BoxMin = BottomVerts[0];
							FVector BoxMax = TopVerts[0];
							for (int i = 1; i < 4; ++i)
							{
								BoxMin = BoxMin.ComponentMin(BottomVerts[i]);
								BoxMin = BoxMin.ComponentMin(TopVerts[i]);
								BoxMax = BoxMax.ComponentMax(BottomVerts[i]);
								BoxMax = BoxMax.ComponentMax(TopVerts[i]);
							}

							// 扩展AABB以包含球体半径
							BoxMin -= FVector(SphereRadius);
							BoxMax += FVector(SphereRadius);

							// 快速拒绝测试
							if (SphereStart.X > BoxMax.X && SphereEnd.X > BoxMax.X) return false;
							if (SphereStart.X < BoxMin.X && SphereEnd.X < BoxMin.X) return false;
							if (SphereStart.Y > BoxMax.Y && SphereEnd.Y > BoxMax.Y) return false;
							if (SphereStart.Y < BoxMin.Y && SphereEnd.Y < BoxMin.Y) return false;
							if (SphereStart.Z > BoxMax.Z && SphereEnd.Z > BoxMax.Z) return false;
							if (SphereStart.Z < BoxMin.Z && SphereEnd.Z < BoxMin.Z) return false;

							// 2. 分离轴定理(SAT)测试
							// 定义长方体的边
							const FVector BottomEdges[4] =
							{
								BottomVerts[1] - BottomVerts[0],
								BottomVerts[2] - BottomVerts[1],
								BottomVerts[3] - BottomVerts[2],
								BottomVerts[0] - BottomVerts[3]
							};

							const FVector SideEdges[4] =
							{
								TopVerts[0] - BottomVerts[0],
								TopVerts[1] - BottomVerts[1],
								TopVerts[2] - BottomVerts[2],
								TopVerts[3] - BottomVerts[3]
							};

							// 测试方向包括: 3个坐标轴、4个底面边与胶囊方向的叉积、4个侧边与胶囊方向的叉积
							const FVector TestDirs[11] =
							{
								FVector(1, 0, 0),  // X轴
								FVector(0, 1, 0),  // Y轴
								FVector(0, 0, 1),  // Z轴
								FVector::CrossProduct(BottomEdges[0], SphereDir).GetSafeNormal(),
								FVector::CrossProduct(BottomEdges[1], SphereDir).GetSafeNormal(),
								FVector::CrossProduct(BottomEdges[2], SphereDir).GetSafeNormal(),
								FVector::CrossProduct(BottomEdges[3], SphereDir).GetSafeNormal(),
								FVector::CrossProduct(SideEdges[0], SphereDir).GetSafeNormal(),
								FVector::CrossProduct(SideEdges[1], SphereDir).GetSafeNormal(),
								FVector::CrossProduct(SideEdges[2], SphereDir).GetSafeNormal(),
								FVector::CrossProduct(SideEdges[3], SphereDir).GetSafeNormal()
							};

							for (const FVector& Axis : TestDirs)
							{
								if (Axis.IsNearlyZero()) continue;

								// 计算长方体在轴上的投影范围
								float BoxMinProj = FLT_MAX, BoxMaxProj = -FLT_MAX;
								for (int i = 0; i < 4; ++i)
								{
									float BottomProj = FVector::DotProduct(BottomVerts[i], Axis);
									float TopProj = FVector::DotProduct(TopVerts[i], Axis);
									BoxMinProj = FMath::Min(BoxMinProj, FMath::Min(BottomProj, TopProj));
									BoxMaxProj = FMath::Max(BoxMaxProj, FMath::Max(BottomProj, TopProj));
								}

								// 计算胶囊在轴上的投影范围
								float CapsuleProj1 = FVector::DotProduct(SphereStart, Axis);
								float CapsuleProj2 = FVector::DotProduct(SphereEnd, Axis);
								float CapsuleMin = FMath::Min(CapsuleProj1, CapsuleProj2) - SphereRadius;
								float CapsuleMax = FMath::Max(CapsuleProj1, CapsuleProj2) + SphereRadius;

								// 检查是否分离
								if (CapsuleMax < BoxMinProj || CapsuleMin > BoxMaxProj)
								{
									return false;
								}
							}

							return true;
						};

					if (SphereIntersectsBox(Start, End, Radius, BottomVertices, TopVertices))
					{
						// 使用距离平方避免开方计算
						const float DistSqr = FVector::DistSquared(Start, FVector(GridData.Location));
						ProcessObstacle(GridData, DistSqr);
					}
				}
			};

		// 检查静态/动态盒体障碍物
		CheckBoxCollision(Cell.BoxObstacles);
		CheckBoxCollision(Cell.BoxObstaclesStatic);

		// ============== 新增提前退出判断 ==============
		if (Hit) // 只有发现过障碍物才需要判断
		{
			const FVector CellCenter = CageToWorld(CellPos);
			const float DistToSegmentSq = FMath::PointDistToSegmentSquared(CellCenter, Start, End);
			const float MinPossibleDist = FMath::Sqrt(DistToSegmentSq) - CellMaxRadius;

			if (MinPossibleDist > 0 && (MinPossibleDist * MinPossibleDist) > ClosestHitDistSq)
			{
				break; // 后续单元不可能更近，提前退出循环
			}
		}
	}
}


// To Do : 1.Sphere Trace For Subjects(can filter by direction angle)  2.Sphere Sweep For Subjects  3.Sphere Sweep For Subjects Async  4.Sector Trace For Subjects  5.Sector Trace For Subjects Async 
// 
// 1. IgnoreList  2.VisibilityCheck  3.AngleCheck  4.KeepCount  5.SortByDist  6.Async


//--------------------------------------------Avoidance---------------------------------------------------------------

void UNeighborGridComponent::Update()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("RVO2 Update");

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("ResetCells");

		ParallelFor(OccupiedCellsQueues.Num(), [&](int32 Index)
		{
			int32 CellIndex;

			while (OccupiedCellsQueues[Index].Dequeue(CellIndex))
			{
				auto& Cell = Cells[CellIndex];
				Cell.Lock();
				Cell.Reset();
				Cell.Unlock();
			}			
		});
	}

	AMechanism* Mechanism = GetMechanism();

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("RegisterSubject");

		auto Chain = Mechanism->EnchainSolid(RegisterSubjectFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently([&](FSolidSubjectHandle Subject, FLocated& Located, FCollider& Collider, FTrace& Trace, FGridData& GridData)
			{
				const auto& Location = Located.Location;

				if (UNLIKELY(!IsInside(Location))) return;

				Trace.Lock();
				Trace.NeighborGrid = this;// when agent traces, it uses this neighbor grid instance.
				Trace.Unlock();

				GridData.Location = FVector3f(Location);
				GridData.Radius = Collider.Radius;

				if (LIKELY(Subject.HasTrait<FAvoidance>()) && LIKELY(Subject.HasTrait<FAvoiding>()))
				{
					auto& Avoidance = Subject.GetTraitRef<FAvoidance>();
					auto& Avoiding = Subject.GetTraitRef<FAvoiding>();

					Avoiding.Position = RVO::Vector2(Location.X, Location.Y);
					Avoiding.Radius = Collider.Radius * Avoidance.AvoidDistMult;

					if (LIKELY(Subject.HasTrait<FMoving>()))
					{
						auto& Moving = Subject.GetTraitRef<FMoving>();
						Avoiding.bCanAvoid = !Moving.bLaunching && !Moving.bPushedBack;
					}
				}

				bool bHasRegisterMultiple = Subject.HasTrait<FRegisterMultiple>();

				if (bHasRegisterMultiple)
				{
					bool bShouldRegister = false;

					auto& Cell = At(Location);

					Cell.Lock();

					if (!Cell.Registered)
					{
						bShouldRegister = true;
						Cell.Registered = true;
					}

					Cell.Subjects.Add(GridData);

					Cell.Unlock();

					if (bShouldRegister)
					{
						int32 CellIndex = GetIndexAt(WorldToCage(Location));
						OccupiedCellsQueues[CellIndex % MaxThreadsAllowed].Enqueue(CellIndex);
					}
				}
				else
				{
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

								bool bShouldRegister = false;

								auto& Cell = At(CurrentCellPos);

								Cell.Lock();

								if (!Cell.Registered)
								{
									bShouldRegister = true;
									Cell.Registered = true;
								}

								Cell.Subjects.Add(GridData);

								Cell.Unlock();

								if (bShouldRegister)
								{
									int32 CellIndex = GetIndexAt(CurrentCellPos);
									OccupiedCellsQueues[CellIndex % MaxThreadsAllowed].Enqueue(CellIndex);
								}
							}
						}
					}
				}

			}, ThreadsCount, BatchSize);
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("RegisterSphereObstacles");

		auto Chain = Mechanism->EnchainSolid(RegisterSphereObstaclesFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, MinBatchSizeAllowed, ThreadsCount, BatchSize);

		Chain->OperateConcurrently([&](FSolidSubjectHandle Subject, FSphereObstacle& SphereObstacle, FLocated& Located, FCollider& Collider, FAvoidance& Avoidance, FAvoiding& Avoiding, FGridData& GridData)
		{
			if (SphereObstacle.bStatic && SphereObstacle.bRegistered) return; // if static, we only register once

			const auto& Location = Located.Location;

			if (UNLIKELY(!IsInside(Location))) return;

			SphereObstacle.Lock();
			SphereObstacle.NeighborGrid = this;// when sphere obstacle override speed limit, it uses this neighbor grid instance.
			SphereObstacle.Unlock();

			GridData.Location = FVector3f(Location);
			GridData.Radius = Collider.Radius;

			Avoiding.Position = RVO::Vector2(Location.X, Location.Y);
			Avoiding.Radius = Collider.Radius * Avoidance.AvoidDistMult;

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

						bool bShouldRegister = false;

						auto& Cell = At(CurrentCellPos);

						Cell.Lock();

						if (!Cell.Registered)
						{
							bShouldRegister = true;
							Cell.Registered = true;
						}

						if (SphereObstacle.bStatic)
						{
							Cell.SphereObstaclesStatic.Add(GridData);
						}
						else
						{
							Cell.SphereObstacles.Add(GridData);
						}

						Cell.Unlock();

						if (bShouldRegister)
						{
							int32 CellIndex = GetIndexAt(CurrentCellPos);
							OccupiedCellsQueues[CellIndex % MaxThreadsAllowed].Enqueue(CellIndex);
						}
					}
				}
			}

			SphereObstacle.bRegistered = true;

		}, ThreadsCount, BatchSize);
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("RegisterBoxObstacles");

		auto Chain = Mechanism->EnchainSolid(RegisterBoxObstaclesFilter);
		UBattleFrameFunctionLibraryRT::CalculateThreadsCountAndBatchSize(Chain->IterableNum(), MaxThreadsAllowed, 1, ThreadsCount, BatchSize);

		Chain->OperateConcurrently([&](FSolidSubjectHandle Subject, FBoxObstacle& BoxObstacle, FGridData& GridData)
		{
			if (BoxObstacle.bStatic && BoxObstacle.bRegistered) return; // if static, we only register once

			const auto& Location = BoxObstacle.point3d_;
			GridData.Location = FVector3f(Location);

			const FBoxObstacle* PreObstaclePtr = BoxObstacle.prevObstacle_.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();
			const FBoxObstacle* NextObstaclePtr = BoxObstacle.nextObstacle_.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();

			if (NextObstaclePtr == nullptr || PreObstaclePtr == nullptr) return;

			const FVector& NextLocation = NextObstaclePtr->point3d_;
			const float ObstacleHeight = BoxObstacle.height_;

			const float StartZ = Location.Z;
			const float EndZ = StartZ + ObstacleHeight;
			TSet<FIntVector> AllGridCells;
			TArray<FIntVector> AllGridCellsArray;

			float CurrentLayerZ = StartZ;

			// 使用CellSize.Z作为Z轴步长
			while (CurrentLayerZ < EndZ)
			{
				const FVector LayerCurrentPoint = FVector(Location.X, Location.Y, CurrentLayerZ);
				const FVector LayerNextPoint = FVector(NextLocation.X, NextLocation.Y, CurrentLayerZ);

				// 使用最大轴尺寸的2倍作为扫描半径
				const float SweepRadius = CellSize.GetMax() * 2.0f;
				auto LayerCells = SphereSweepForCells(LayerCurrentPoint, LayerNextPoint, SweepRadius);

				for (const auto& CellPos : LayerCells)
				{
					AllGridCells.Add(CellPos);
				}

				CurrentLayerZ += CellSize.Z; // 使用Z轴尺寸作为步长
			}

			AllGridCellsArray = AllGridCells.Array();

			ParallelFor(AllGridCellsArray.Num(), [&](int32 Index)
				{
					const FIntVector& CellPos = AllGridCellsArray[Index];

					bool bShouldRegister = false;

					auto& Cell = At(CellPos);

					Cell.Lock();

					if (!Cell.Registered)
					{
						bShouldRegister = true;
						Cell.Registered = true;
					}

					if (BoxObstacle.bStatic)
					{
						Cell.BoxObstaclesStatic.Add(GridData);
					}
					else
					{
						Cell.BoxObstacles.Add(GridData);
					}

					Cell.Unlock();

					if (bShouldRegister)
					{
						int32 CellIndex = GetIndexAt(CellPos);
						OccupiedCellsQueues[CellIndex % MaxThreadsAllowed].Enqueue(CellIndex);
					}
				});

			BoxObstacle.bRegistered = true;

		}, ThreadsCount, BatchSize);
	}

}

