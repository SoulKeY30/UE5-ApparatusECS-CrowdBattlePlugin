#pragma once
 
#include "CoreMinimal.h"
#include "RoadBlock.generated.h"

class UNeighborGridComponent;

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FRoadBlock
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

public:

	bool bOverrideSpeedLimit = true;
	float NewSpeedLimit = 0;
	TSet<FSubjectHandle> OverridingAgents;
	UNeighborGridComponent* NeighborGrid = nullptr;

	FRoadBlock() {};

	FRoadBlock(const FRoadBlock& RoadBlock)
	{
		LockFlag.store(RoadBlock.LockFlag.load());
		NeighborGrid = RoadBlock.NeighborGrid;
		bOverrideSpeedLimit = RoadBlock.bOverrideSpeedLimit;
		NewSpeedLimit = RoadBlock.NewSpeedLimit;
		OverridingAgents = RoadBlock.OverridingAgents;
	}

	FRoadBlock& operator=(const FRoadBlock& RoadBlock)
	{
		LockFlag.store(RoadBlock.LockFlag.load());
		NeighborGrid = RoadBlock.NeighborGrid;
		bOverrideSpeedLimit = RoadBlock.bOverrideSpeedLimit;
		NewSpeedLimit = RoadBlock.NewSpeedLimit;
		OverridingAgents = RoadBlock.OverridingAgents;

		return *this;
	}

};
