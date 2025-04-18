#pragma once

#include "CoreMinimal.h"
#include "Burning.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FBurning
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

	int32 RemainingTemporalDamager = 1;

	FBurning() {};

	FBurning(const FBurning& Burning)
	{
		LockFlag.store(Burning.LockFlag.load());
		RemainingTemporalDamager = Burning.RemainingTemporalDamager;
	}

	FBurning& operator=(const FBurning& Burning)
	{
		LockFlag.store(Burning.LockFlag.load());
		RemainingTemporalDamager = Burning.RemainingTemporalDamager;
		return *this;
	}
};