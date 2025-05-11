#pragma once
 
#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "SphereObstacle.generated.h"

class UNeighborGridComponent;

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSphereObstacle
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
	bool bStatic = false;
	bool bRegistered = false;
	bool bExcluded = false;

	FSphereObstacle() {};

	FSphereObstacle(const FSphereObstacle& SphereObstacle)
	{
		LockFlag.store(SphereObstacle.LockFlag.load());
		NeighborGrid = SphereObstacle.NeighborGrid;
		bOverrideSpeedLimit = SphereObstacle.bOverrideSpeedLimit;
		NewSpeedLimit = SphereObstacle.NewSpeedLimit;
		OverridingAgents = SphereObstacle.OverridingAgents;
		bStatic = SphereObstacle.bStatic;
		bRegistered = SphereObstacle.bRegistered;
		bExcluded = SphereObstacle.bExcluded;
	}

	FSphereObstacle& operator=(const FSphereObstacle& SphereObstacle)
	{
		LockFlag.store(SphereObstacle.LockFlag.load());
		NeighborGrid = SphereObstacle.NeighborGrid;
		bOverrideSpeedLimit = SphereObstacle.bOverrideSpeedLimit;
		NewSpeedLimit = SphereObstacle.NewSpeedLimit;
		OverridingAgents = SphereObstacle.OverridingAgents;
		bStatic = SphereObstacle.bStatic;
		bRegistered = SphereObstacle.bRegistered;
		bExcluded = SphereObstacle.bExcluded;

		return *this;
	}

};
