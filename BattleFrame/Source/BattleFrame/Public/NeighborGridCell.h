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
#include "Machine.h"
#include "Traits/GridData.h"
#include "NeighborGridCell.generated.h"
    
 /**
  * Struct representing a one grid cell.
  */
USTRUCT(BlueprintType, Category = "NeighborGrid")
struct BATTLEFRAME_API FNeighborGridCell
{
	GENERATED_BODY()

private:
 
	mutable std::atomic<bool> LockFlag{ false };

public:

	void Lock() const
	{
		while (LockFlag.exchange(true, std::memory_order_acquire));
	}

	void Unlock() const
	{
		LockFlag.store(false, std::memory_order_release);
	}


	TArray<FGridData, TInlineAllocator<8>> Subjects;
	TArray<FGridData, TInlineAllocator<8>> SphereObstacles;
	TArray<FGridData, TInlineAllocator<8>> BoxObstacles;
	TArray<FGridData, TInlineAllocator<8>> SphereObstaclesStatic;
	TArray<FGridData, TInlineAllocator<8>> BoxObstaclesStatic;

	bool Registered = false;

	FNeighborGridCell(){}

	FNeighborGridCell(const FNeighborGridCell& Cell)
	{
		LockFlag.store(Cell.LockFlag.load());

		Registered = Cell.Registered;
		Subjects = Cell.Subjects;
		SphereObstacles = Cell.SphereObstacles;
		BoxObstacles = Cell.BoxObstacles;
		SphereObstaclesStatic = Cell.SphereObstaclesStatic;
		BoxObstaclesStatic = Cell.BoxObstaclesStatic;
	}

	FNeighborGridCell& operator=(const FNeighborGridCell& Cell)
	{
		Registered = Cell.Registered;
		Subjects = Cell.Subjects;
		SphereObstacles = Cell.SphereObstacles;
		BoxObstacles = Cell.BoxObstacles;
		SphereObstaclesStatic = Cell.SphereObstaclesStatic;
		BoxObstaclesStatic = Cell.BoxObstaclesStatic;

		return *this;
	}

	void Reset()
	{
		Registered = false;

		// 清空所有存储的障碍物数据
		Subjects.Empty();
		SphereObstacles.Empty();
		BoxObstacles.Empty();
	}
};
