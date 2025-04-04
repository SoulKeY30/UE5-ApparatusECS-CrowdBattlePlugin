#pragma once

#include "CoreMinimal.h"
#include "OccupiedCells.generated.h"


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FOccupiedCells
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

	TSet<int32> Cells;

	FOccupiedCells() {};

	FOccupiedCells(const FOccupiedCells& OccupiedCells)
	{
		LockFlag.store(OccupiedCells.LockFlag.load());
		Cells = OccupiedCells.Cells;
	}

	FOccupiedCells& operator=(const FOccupiedCells& OccupiedCells)
	{
		LockFlag.store(OccupiedCells.LockFlag.load());
		Cells = OccupiedCells.Cells;
		return *this;
	}
};
