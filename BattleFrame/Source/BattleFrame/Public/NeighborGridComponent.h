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

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MechanicalActorComponent.h"
#include "Machine.h"
#include "NeighborGridCell.h"
#include "Traits/Avoidance.h"
#include "BattleFrameEnums.h"
#include "BattleFrameStructs.h"
#include "RVOSimulator.h"
#include "RVOVector2.h"
#include "RVODefinitions.h"

#include "NeighborGridComponent.generated.h"

#define BUBBLE_DEBUG 0

class ANeighborGridActor;

// 表示运动路径的胶囊体
struct FCapsulePath
{
	FVector Start;
	FVector End;
	float Radius;

	FCapsulePath(const FVector& InStart, const FVector& InEnd, float InRadius) : Start(InStart), End(InEnd), Radius(InRadius) {}
};

UCLASS(Category = "NeighborGrid")
class BATTLEFRAME_API UNeighborGridComponent : public UMechanicalActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Performance)
	int32 MaxThreadsAllowed = FMath::Clamp(FPlatformMisc::NumberOfWorkerThreadsToSpawn(), 1, 20);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Performance)
	int32 MinBatchSizeAllowed = 100;

	int32 ThreadsCount = 1;
	int32 BatchSize = 1;

	#if WITH_EDITORONLY_DATA
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debugging")
	bool bDebugDrawCageCells = false;
	#endif

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grid")
	FVector CellSize = FVector(300.f,300.f,300.f);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grid")
	FIntVector GridSize = FIntVector(20, 20, 1);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Grid")
	mutable FBox Bounds;

	TArray<FNeighborGridCell> Cells;
	FVector InvCellSizeCache = FVector(1 / 300.f, 1 / 300.f, 1 / 300.f);
	TArray<TQueue<int32,EQueueMode::Mpsc>> OccupiedCellsQueues;

	FFilter RegisterNeighborGrid_Trace_Filter;
	FFilter RegisterNeighborGrid_SphereObstacle_Filter;
	FFilter RegisterSubjectSingleFilter;
	FFilter RegisterSubjectMultipleFilter;
	FFilter RegisterSphereObstaclesFilter;
	FFilter RegisterBoxObstaclesFilter;
	FFilter SubjectFilterBase;
	FFilter SphereObstacleFilter;
	FFilter BoxObstacleFilter;
	FFilter DecoupleFilter;


	//---------------------------------------------Init------------------------------------------------------------------

	UNeighborGridComponent();

	void InitializeComponent() override;

	void DoInitializeCells()
	{
		Cells.Reset(); // Make sure there are no cells.
		Cells.AddDefaulted(GridSize.X * GridSize.Y * GridSize.Z);
		OccupiedCellsQueues.SetNum(MaxThreadsAllowed);
		InvCellSizeCache = FVector(1 / CellSize.X, 1 / CellSize.Y, 1 / CellSize.Z);
	}

	void BeginPlay() override;


	//---------------------------------------------Tracing------------------------------------------------------------------

	void SphereTraceForSubjects
	(
		const int32 KeepCount,
		const FVector& Origin, 
		const float Radius, 
		const bool bCheckVisibility, 
		const FVector& CheckOrigin, 
		const float CheckRadius, 
		const ESortMode SortMode,
		const FVector& SortOrigin,
		const FSubjectArray& IgnoreSubjects,
		const FFilter& Filter, 
		bool& Hit, 
		TArray<FTraceResult>& Results
	) const;

	void SphereSweepForSubjects
	(
		const int32 KeepCount,
		const FVector& Start, 
		const FVector& End, 
		const float Radius, 
		const bool bCheckVisibility, 
		const FVector& CheckOrigin, 
		const float CheckRadius, 
		const ESortMode SortMode,
		const FVector& SortOrigin,
		const FSubjectArray& IgnoreSubjects,
		const FFilter& Filter, 
		bool& Hit, 
		TArray<FTraceResult>& Results
	) const;

	void SectorTraceForSubjects
	(
		const int32 KeepCount,
		const FVector& Origin, 
		const float Radius, 
		const float Height, 
		const FVector& Direction, 
		const float Angle, 
		const bool bCheckVisibility, 
		const FVector& CheckOrigin, 
		const float CheckRadius, 
		const ESortMode SortMode,
		const FVector& SortOrigin, 
		const FSubjectArray& IgnoreSubjects,
		const FFilter& Filter, 
		bool& Hit, 
		TArray<FTraceResult>& Results
	) const;	

	void SphereSweepForObstacle
	(
		const FVector& Start,
		const FVector& End,
		float Radius,
		bool& Hit,
		FTraceResult& Result
	) const;

	void Update();
	void Decouple();
	void Evaluate();

	void DefineFilters();

	//---------------------------------------------RVO2------------------------------------------------------------------

	void ComputeNewVelocity(FAvoidance& Avoidance, const TArray<FAvoiding>& SubjectNeighbors, const TArray<FAvoiding>& ObstacleNeighbors, float timeStep_);

	FORCEINLINE bool LinearProgram1(const std::vector<RVO::Line>& lines, size_t lineNo, float radius, const RVO::Vector2& optVelocity, bool directionOpt, RVO::Vector2& result)
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

	FORCEINLINE size_t LinearProgram2(const std::vector<RVO::Line>& lines, float radius, const RVO::Vector2& optVelocity, bool directionOpt, RVO::Vector2& result)
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

	FORCEINLINE void LinearProgram3(const std::vector<RVO::Line>& lines, size_t numObstLines, size_t beginLine, float radius, RVO::Vector2& result)
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

	FORCEINLINE TArray<FIntVector> GetNeighborCells(const FVector& Center, const FVector& Range3D) const
	{
		//TRACE_CPUPROFILER_EVENT_SCOPE_STR("GetNeighborCells");

		const FIntVector Min = WorldToCage(Center - Range3D);
		const FIntVector Max = WorldToCage(Center + Range3D);

		TArray<FIntVector> ValidCells;
		const int32 ExpectedCells = (Max.X - Min.X + 1) * (Max.Y - Min.Y + 1) * (Max.Z - Min.Z + 1);
		ValidCells.Reserve(ExpectedCells); // 根据场景规模调整

		for (int32 z = Min.Z; z <= Max.Z; ++z) 
		{
			for (int32 y = Min.Y; y <= Max.Y; ++y) 
			{
				for (int32 x = Min.X; x <= Max.X; ++x) 
				{
					const FIntVector CellPos(x, y, z);
					if (LIKELY(IsInside(CellPos))) 
					{ // 分支预测优化
						ValidCells.Emplace(CellPos);
					}
				}
			}
		}
		return ValidCells;
	}

	FORCEINLINE TArray<FIntVector> SphereSweepForCells(const FVector& Start, const FVector& End, float Radius) const
	{
		//TRACE_CPUPROFILER_EVENT_SCOPE_STR("SphereSweepForCells");

		// 预计算关键参数 - 现在每个轴有自己的半径值
		const FVector RadiusInCellsValue(Radius / CellSize.X, Radius / CellSize.Y, Radius / CellSize.Z);
		const FIntVector RadiusInCells(
			FMath::CeilToInt(RadiusInCellsValue.X),
			FMath::CeilToInt(RadiusInCellsValue.Y),
			FMath::CeilToInt(RadiusInCellsValue.Z));
		const FVector RadiusSq(
			FMath::Square(RadiusInCellsValue.X),
			FMath::Square(RadiusInCellsValue.Y),
			FMath::Square(RadiusInCellsValue.Z));

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

	FORCEINLINE void AddSphereCells(const FIntVector& CenterCell, const FIntVector& RadiusInCells, const FVector& RadiusSq, TSet<FIntVector>& GridCells) const
	{
		//TRACE_CPUPROFILER_EVENT_SCOPE_STR("AddSphereCells");

		// 分层遍历优化：减少无效循环
		for (int32 x = -RadiusInCells.X; x <= RadiusInCells.X; ++x)
		{
			const float XNormSq = FMath::Square(static_cast<float>(x) / RadiusInCells.X);
			if (XNormSq > 1.0f) continue;

			for (int32 y = -RadiusInCells.Y; y <= RadiusInCells.Y; ++y)
			{
				const float YNormSq = FMath::Square(static_cast<float>(y) / RadiusInCells.Y);
				if (XNormSq + YNormSq > 1.0f) continue;

				// 直接计算z范围，减少循环次数
				const float RemainingNormSq = 1.0f - (XNormSq + YNormSq);
				const int32 MaxZ = FMath::FloorToInt(RadiusInCells.Z * FMath::Sqrt(RemainingNormSq));
				for (int32 z = -MaxZ; z <= MaxZ; ++z)
				{
					GridCells.Add(CenterCell + FIntVector(x, y, z));
				}
			}
		}
	}

	/* Get the size of a single cell in global units.*/
	FORCEINLINE const auto GetCellSize() const
	{
		return CellSize;
	}

	/* Get the size of the cage in cells among each axis.*/
	FORCEINLINE const FIntVector& GetSize() const
	{
		return GridSize;
	}

	/* Get the global bounds of the cage in world units.*/
	FORCEINLINE const FBox& GetBounds() const
	{
		const auto Extents = CellSize * FVector(GridSize) * 0.5f;
		const auto Actor = GetOwner();
		const auto Location = (Actor != nullptr) ? Actor->GetActorLocation() : FVector::ZeroVector;
		Bounds.Min = Location - Extents;
		Bounds.Max = Location + Extents;
		return Bounds;
	}

	/* Convert a cage-local 3D location to a global 3D location. No bounding checks are performed.*/
	FORCEINLINE FVector CageToWorld(const FIntVector& CagePoint) const
	{
		// Convert the cage point to a local position within the cage
		FVector LocalPoint = FVector(CagePoint.X, CagePoint.Y, CagePoint.Z) * CellSize;

		// Convert the local position to a global position by adding the cage's minimum bounds
		return LocalPoint + Bounds.Min;
	}

	/* Convert a global 3D location to a position within the cage.mNo bounding checks are performed.*/
	FORCEINLINE FIntVector WorldToCage(FVector Point) const
	{
		Point -= Bounds.Min;
		Point *= InvCellSizeCache;
		return FIntVector(FMath::FloorToInt(Point.X), FMath::FloorToInt(Point.Y), FMath::FloorToInt(Point.Z));
	}

	/* Convert a global 3D location to a position within the bounds. No bounding checks are performed.*/
	FORCEINLINE FVector WorldToBounded(const FVector& Point) const
	{
		return Point - Bounds.Min;
	}

	/* Convert a cage-local 3D location to a position within the cage. No bounding checks are performed.*/
	FORCEINLINE FIntVector BoundedToCage(FVector Point) const
	{
		Point *= InvCellSizeCache;
		return FIntVector(FMath::FloorToInt(Point.X), FMath::FloorToInt(Point.Y), FMath::FloorToInt(Point.Z));
	}

	/* Get the index of the cage cell.*/
	FORCEINLINE int32 GetIndexAt(int32 X, int32 Y, int32 Z) const
	{
		X = FMath::Clamp(X, 0, GridSize.X - 1);
		Y = FMath::Clamp(Y, 0, GridSize.Y - 1);
		Z = FMath::Clamp(Z, 0, GridSize.Z - 1);
		return X + GridSize.X * (Y + GridSize.Y * Z);
	}

	/* Get a position within the cage by an index of the cell.*/
	FORCEINLINE FIntVector GetCellPointByIndex(int32 Index) const
	{
		int32 z = Index / (GridSize.X * GridSize.Y);
		int32 LayerPadding = Index - (z * GridSize.X * GridSize.Y);

		return FIntVector(LayerPadding / GridSize.X, LayerPadding % GridSize.X, z);
	}

	/* Get the index of the cage cell. */
	FORCEINLINE int32 GetIndexAt(const FIntVector& CellPoint) const
	{
		return GetIndexAt(CellPoint.X, CellPoint.Y, CellPoint.Z);
	}

	/* Get the index of the cell by the world position. */
	FORCEINLINE int32 GetIndexAt(const FVector& Point) const
	{
		return GetIndexAt(WorldToCage(Point));
	}

	/* Get subjects in a specific cage cell. Const version. */
	FORCEINLINE const FNeighborGridCell& At(const int32 X, const int32 Y, const int32 Z) const
	{
		return Cells[GetIndexAt(X, Y, Z)];
	}

	/* Get subjects in a specific cage cell. */
	FORCEINLINE FNeighborGridCell& At(const int32 X, const int32 Y, const int32 Z)
	{
		return Cells[GetIndexAt(X, Y, Z)];
	}

	/* Check if the cage point is inside the cage. */
	FORCEINLINE bool IsInside(const FIntVector& CellPoint) const
	{
		//TRACE_CPUPROFILER_EVENT_SCOPE_STR("IsInside");
		return (CellPoint.X >= 0) && (CellPoint.X < GridSize.X) && (CellPoint.Y >= 0) && (CellPoint.Y < GridSize.Y) && (CellPoint.Z >= 0) && (CellPoint.Z < GridSize.Z);
	}

	/* Check if the world point is inside the cage. */
	FORCEINLINE bool IsInside(const FVector& WorldPoint) const
	{
		return IsInside(WorldToCage(WorldPoint));
	}

	/* Get subjects in a specific cage cell by position in the cage. */
	FORCEINLINE FNeighborGridCell& At(const FIntVector& CellPoint)
	{
		return At(CellPoint.X, CellPoint.Y, CellPoint.Z);
	}

	/* Get subjects in a specific cage cell by position in the cage. */
	FORCEINLINE const FNeighborGridCell& At(const FIntVector& CellPoint) const
	{
		return At(CellPoint.X, CellPoint.Y, CellPoint.Z);
	}

	/* Get subjects in a specific cage cell by world 3d-location. */
	FORCEINLINE FNeighborGridCell& At(FVector Point)
	{
		return At(WorldToCage(Point));
	}

	/* Get subjects in a specific cage cell by world 3d-location. */
	FORCEINLINE const FNeighborGridCell& At(FVector Point) const
	{
		return At(WorldToCage(Point));
	}

	/* Get a box shape representing a cell by position in the cage. */
	FORCEINLINE FBox BoxAt(const FIntVector& CellPoint)
	{
		const FVector Min = Bounds.Min + FVector(CellPoint) * CellSize;
		FBox Box(Min, Min + CellSize);
		return MoveTemp(Box);
	}

	/* Get a box shape representing a cell by world 3d-location. */
	FORCEINLINE FBox BoxAt(const FVector Point)
	{
		return BoxAt(WorldToCage(Point));
	}

};

