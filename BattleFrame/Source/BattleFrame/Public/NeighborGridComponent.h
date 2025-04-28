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
#include "Traits/OccupiedCells.h"
#include "Traits/TraceResult.h"
#include "Traits/BattleFrameEnums.h"
#include "BitMask.h"
#include "RVOSimulator.h"
#include "RVOVector2.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "NeighborGridComponent.generated.h"

#define BUBBLE_DEBUG 0

class ANeighborGridActor;

// 表示运动路径的胶囊体
struct FCapsulePath
{
	FVector Start;
	FVector End;
	float Radius;

	FCapsulePath(const FVector& InStart, const FVector& InEnd, float InRadius)
		: Start(InStart), End(InEnd), Radius(InRadius) {
	}
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
	float CellSize = 300.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grid")
	FIntVector Size = FIntVector(20, 20, 1);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Grid")
	mutable FBox Bounds;

	TArray<FNeighborGridCell> Cells;
	float InvCellSizeCache = 1/ 300;

	TArray<TQueue<int32,EQueueMode::Mpsc>> OccupiedCellsQueues;

	UNeighborGridComponent();

	void InitializeComponent() override;

	void BeginPlay() override;

	void BeginDestroy() override;

	void DoInitializeCells()
	{
		Cells.Reset(); // Make sure there are no cells.
		Cells.AddDefaulted(Size.X * Size.Y * Size.Z);
		OccupiedCellsQueues.SetNum(MaxThreadsAllowed);
	}

	void SphereTraceForSubjects
	(
		const FVector& Origin, 
		const float Radius, 
		const bool bCheckVisibility, 
		const FVector& CheckOrigin, 
		const float CheckRadius, 
		const TArray<FSubjectHandle>& IgnoreSubjects, 
		const FFilter& Filter, 
		bool& Hit, 
		TArray<FTraceResult>& Results
	) const;

	void SphereSweepForSubjects
	(
		const FVector& Start, 
		const FVector& End, 
		const float Radius, 
		const bool bCheckVisibility, 
		const FVector& CheckOrigin, 
		const float CheckRadius, 
		ESortMode SortMode, 
		const FVector& SortOrigin,
		int32 KeepCount, 
		const TArray<FSubjectHandle>& IgnoreSubjects, 
		const FFilter& Filter, bool& Hit, 
		TArray<FTraceResult>& Results
	) const;

	void SectorTraceForSubjects
	(
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
		int32 KeepCount, 
		const TArray<FSubjectHandle>& IgnoreSubjects, 
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

	void ComputeNewVelocity(FAvoidance& Avoidance, TArray<FAvoiding>& SubjectNeighbors, TArray<FAvoiding>& ObstacleNeighbors, float timeStep_);
	bool LinearProgram1(const std::vector<RVO::Line>& lines, size_t lineNo, float radius, const RVO::Vector2& optVelocity, bool directionOpt, RVO::Vector2& result);
	size_t LinearProgram2(const std::vector<RVO::Line>& lines, float radius, const RVO::Vector2& optVelocity, bool directionOpt, RVO::Vector2& result);
	void LinearProgram3(const std::vector<RVO::Line>& lines, size_t numObstLines, size_t beginLine, float radius, RVO::Vector2& result);

	//---------------------------------------------Helpers------------------------------------------------------------------

	FORCEINLINE TArray<FIntVector> GetNeighborCells(const FVector& Center, const FVector& Range3D) const
	{
		//TRACE_CPUPROFILER_EVENT_SCOPE_STR("GetNeighborCells");

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

	FORCEINLINE TArray<FIntVector> SphereSweepForCells(const FVector& Start, const FVector& End, float Radius) const
	{
		//TRACE_CPUPROFILER_EVENT_SCOPE_STR("SphereSweepForCells");

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

	FORCEINLINE void AddSphereCells(const FIntVector& CenterCell, int32 RadiusInCells, float RadiusSq, TSet<FIntVector>& GridCells) const
	{
		//TRACE_CPUPROFILER_EVENT_SCOPE_STR("AddSphereCells");

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



	/**
	 * Get the size of a single cell in global units.
	 */
	const auto GetCellSize() const
	{
		return CellSize;
	}

	/**
	 * Get the size of the cage in cells among each axis.
	 */
	const FIntVector& GetSize() const
	{
		return Size;
	}

	/**
	 * Get the global bounds of the cage in world units.
	 */
	const FBox& GetBounds() const
	{
		const auto Extents = CellSize * FVector(Size) * 0.5f;
		const auto Actor = GetOwner();
		const auto Location = (Actor != nullptr) ? Actor->GetActorLocation() : FVector::ZeroVector;
		Bounds.Min = Location - Extents;
		Bounds.Max = Location + Extents;
		return Bounds;
	}

	/**
	 * Convert a cage-local 3D location to a global 3D location. No bounding checks are performed.
	 */
	FORCEINLINE FVector CageToWorld(const FIntVector& CagePoint) const
	{
		// Convert the cage point to a local position within the cage
		FVector LocalPoint = FVector(CagePoint.X, CagePoint.Y, CagePoint.Z) * CellSize;

		// Convert the local position to a global position by adding the cage's minimum bounds
		return LocalPoint + Bounds.Min;
	}

	/**
	 * Convert a global 3D location to a position within the cage.mNo bounding checks are performed.
	 */
	FORCEINLINE FIntVector WorldToCage(FVector Point) const
	{
		Point -= Bounds.Min;
		Point *= InvCellSizeCache;
		return FIntVector(FMath::FloorToInt(Point.X),
			FMath::FloorToInt(Point.Y),
			FMath::FloorToInt(Point.Z));
	}

	/**
	 * Convert a global 3D location to a position within the bounds. No bounding checks are performed.
	 */
	FORCEINLINE FVector WorldToBounded(const FVector& Point) const
	{
		return Point - Bounds.Min;
	}

	/**
	 * Convert a cage-local 3D location to a position within the cage. No bounding checks are performed.
	 */
	FORCEINLINE FIntVector BoundedToCage(FVector Point) const
	{
		Point *= InvCellSizeCache;
		return FIntVector(FMath::FloorToInt(Point.X),
			FMath::FloorToInt(Point.Y),
			FMath::FloorToInt(Point.Z));
	}

	/* Get the index of the cage cell. */
	FORCEINLINE int32 GetIndexAt(int32 X, int32 Y, int32 Z) const
	{
		X = FMath::Clamp(X, 0, Size.X - 1);
		Y = FMath::Clamp(Y, 0, Size.Y - 1);
		Z = FMath::Clamp(Z, 0, Size.Z - 1);
		return X + Size.X * (Y + Size.Y * Z);
	}

	/**
	 * Get a position within the cage by an index of the cell.
	 */
	FORCEINLINE FIntVector GetCellPointByIndex(int32 Index) const
	{
		int32 z = Index / (Size.X * Size.Y);
		int32 LayerPadding = Index - (z * Size.X * Size.Y);

		return FIntVector(LayerPadding / Size.X, LayerPadding % Size.X, z);
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
		return (CellPoint.X >= 0) && (CellPoint.X < Size.X) &&
			(CellPoint.Y >= 0) && (CellPoint.Y < Size.Y) &&
			(CellPoint.Z >= 0) && (CellPoint.Z < Size.Z);
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
		FBox Box(Min, Min + FVector(CellSize));
		return MoveTemp(Box);
	}

	/* Get a box shape representing a cell by world 3d-location. */
	FORCEINLINE FBox BoxAt(const FVector Point)
	{
		return BoxAt(WorldToCage(Point));
	}

};

