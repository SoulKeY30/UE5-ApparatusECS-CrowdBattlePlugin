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
#include "BattleFrameEnums.h"
#include "BattleFrameStructs.h"
#include "RVOSimulator.h"
#include "RVOVector2.h"
#include "RVODefinitions.h"
#include "Traits/Avoidance.h"
#include "Traits/RegisterMultiple.h"
#include "Traits/Collider.h"
#include "Traits/Located.h"
#include "Traits/BoxObstacle.h"
#include "Traits/Moving.h"
#include "Traits/SphereObstacle.h"
#include "Traits/Appearing.h"
#include "Traits/Move.h"
#include "Traits/Trace.h"
#include "Traits/GridData.h"
#include "Traits/Avoiding.h"
#include "Traits/Corpse.h"
#include "Traits/Dying.h"
#include "Traits/Activated.h"
#include "Traits/Agent.h"
#include "Traits/Patrol.h"
#include "Traits/Directed.h"
#include "Math/UnrealMathUtility.h"
#include "NeighborGridComponent.generated.h"

#define BUBBLE_DEBUG 0

class ANeighborGridActor;


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

	TArray<FNeighborGridCell> SubjectCells;
	TArray<FNeighborGridCell> ObstacleCells;
	TArray<FNeighborGridCell> StaticObstacleCells;

	FVector InvCellSizeCache = FVector(1 / 300.f, 1 / 300.f, 1 / 300.f);
	TArray<TQueue<int32,EQueueMode::Mpsc>> OccupiedCellsQueues;

	EFlagmarkBit RegisterMultipleFlag = EFlagmarkBit::M;

	FFilter RegisterNeighborGrid_Trace_Filter;
	FFilter RegisterNeighborGrid_SphereObstacle_Filter;
	FFilter RegisterSubjectFilter;
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
		SubjectCells.Empty();
		ObstacleCells.Empty();
		StaticObstacleCells.Empty();

		OccupiedCellsQueues.Empty();

		SubjectCells.AddDefaulted(GridSize.X * GridSize.Y * GridSize.Z);
		ObstacleCells.AddDefaulted(GridSize.X * GridSize.Y * GridSize.Z);
		StaticObstacleCells.AddDefaulted(GridSize.X * GridSize.Y * GridSize.Z);

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

	void DefineFilters();


	//---------------------------------------------Helpers------------------------------------------------------------------

	FORCEINLINE TArray<FIntVector> GetNeighborCells(const FVector& Center, const FVector& Range3D) const
	{
		const FIntVector Min = LocationToCoord(Center - Range3D);
		const FIntVector Max = LocationToCoord(Center + Range3D);

		TArray<FIntVector> ValidCells;
		const int32 ExpectedCells = (Max.X - Min.X + 1) * (Max.Y - Min.Y + 1) * (Max.Z - Min.Z + 1);
		ValidCells.Reserve(ExpectedCells); // 根据场景规模调整

		for (int32 z = Min.Z; z <= Max.Z; ++z) 
		{
			for (int32 y = Min.Y; y <= Max.Y; ++y) 
			{
				for (int32 x = Min.X; x <= Max.X; ++x) 
				{
					const FIntVector Coord(x, y, z);
					if (LIKELY(IsInside(Coord))) 
					{ // 分支预测优化
						ValidCells.Emplace(Coord);
					}
				}
			}
		}
		return ValidCells;
	}

	FORCEINLINE TArray<FIntVector> SphereSweepForCells(const FVector& Start, const FVector& End, float Radius) const
	{
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

		FIntVector StartCell = LocationToCoord(Start);
		FIntVector EndCell = LocationToCoord(End);
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
				const FVector WorldA = CoordToLocation(A);
				const FVector WorldB = CoordToLocation(B);
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

	/* Check if the cage point is inside the cage. */
	FORCEINLINE bool IsInside(const FIntVector& Coord) const
	{
		return (Coord.X >= 0) && (Coord.X < GridSize.X) && (Coord.Y >= 0) && (Coord.Y < GridSize.Y) && (Coord.Z >= 0) && (Coord.Z < GridSize.Z);
	}

	/* Check if the world point is inside the cage. */
	FORCEINLINE bool IsInside(const FVector& Location) const
	{
		return IsInside(LocationToCoord(Location));
	}

	/* Convert a cage-local 3D location to a global 3D location. No bounding checks are performed.*/
	FORCEINLINE FVector CoordToLocation(const FIntVector& Coord) const
	{
		// Convert the cage point to a local position within the cage
		FVector LocalPoint = FVector(Coord.X, Coord.Y, Coord.Z) * CellSize;

		// Convert the local position to a global position by adding the cage's minimum bounds
		return LocalPoint + Bounds.Min;
	}

	/* Convert a global 3D location to a position within the cage.No bounding checks are performed.*/
	FORCEINLINE FIntVector LocationToCoord(const FVector& Location) const
	{
		FVector NewLocation = (Location - Bounds.Min) * InvCellSizeCache; 
		return FIntVector(FMath::FloorToInt(NewLocation.X), FMath::FloorToInt(NewLocation.Y), FMath::FloorToInt(NewLocation.Z));
	}

	/* Get the index of the cage cell.*/
	FORCEINLINE int32 CoordToIndex(const FIntVector& Coord) const
	{
		int32 X = FMath::Clamp(Coord.X, 0, GridSize.X - 1);
		int32 Y = FMath::Clamp(Coord.Y, 0, GridSize.Y - 1);
		int32 Z = FMath::Clamp(Coord.Z, 0, GridSize.Z - 1);
		return X + GridSize.X * (Y + GridSize.Y * Z);
	}

	/* Get a position within the cage by an index of the cell.*/
	FORCEINLINE FIntVector IndexToCoord(const int32 Index) const
	{
		int32 Z = Index / (GridSize.X * GridSize.Y);
		int32 LayerPadding = Index - (Z * GridSize.X * GridSize.Y);

		return FIntVector(LayerPadding / GridSize.X, LayerPadding % GridSize.X, Z);
	}

	/* Get the index of the cell by the world position. */
	FORCEINLINE int32 LocationToIndex(const FVector& Location) const
	{
		return CoordToIndex(LocationToCoord(Location));
	}

	/* Get subjects in a specific cage cell by position in the cage. */
	FORCEINLINE FNeighborGridCell& GetCellAt(TArray<FNeighborGridCell>& Cells, const FIntVector& Coord) const
	{
		return Cells[CoordToIndex(Coord)];
	}

	FORCEINLINE const FNeighborGridCell& GetCellAt(const TArray<FNeighborGridCell>& Cells, const FIntVector& Coord) const
	{
		return Cells[CoordToIndex(Coord)];
	}

	/* Get subjects in a specific cage cell by world 3d-location. */
	FORCEINLINE FNeighborGridCell& GetCellAt(TArray<FNeighborGridCell>& Cells, const FVector& Location) const
	{
		return Cells[LocationToIndex(Location)];
	}

	FORCEINLINE const FNeighborGridCell& GetCellAt(const TArray<FNeighborGridCell>& Cells, const FVector& Location) const
	{
		return Cells[LocationToIndex(Location)];
	}
};

