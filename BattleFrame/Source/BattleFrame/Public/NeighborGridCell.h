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

	FORCEINLINE void Lock() const
	{
		while (LockFlag.exchange(true, std::memory_order_acquire));
	}

	FORCEINLINE void Unlock() const
	{
		LockFlag.store(false, std::memory_order_release);
	}

	TArray<FGridData, TInlineAllocator<8>> Subjects;
	bool bRegistered = false;

	FORCEINLINE FNeighborGridCell(){}

	FORCEINLINE FNeighborGridCell(const FNeighborGridCell& Cell)
	{
		LockFlag.store(Cell.LockFlag.load());
		Subjects = Cell.Subjects;
		bRegistered = Cell.bRegistered;
	}

	FNeighborGridCell& operator=(const FNeighborGridCell& Cell)
	{
		Subjects = Cell.Subjects;
		bRegistered = Cell.bRegistered;
		return *this;
	}

	FORCEINLINE void Empty()
	{
		Subjects.Empty();
		bRegistered = false;
	}
};
