#pragma once
 
#include "CoreMinimal.h"
#include "Slowing.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSlowing
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

	TSet<FSubjectHandle> Slowers;
	float CombinedSlowMult = 1;

	FSlowing() {};

	FSlowing(const FSlowing& Slowing)
	{
		LockFlag.store(Slowing.LockFlag.load());
		Slowers = Slowing.Slowers;
		CombinedSlowMult = Slowing.CombinedSlowMult;
	}

	FSlowing& operator=(const FSlowing& Slowing)
	{
		LockFlag.store(Slowing.LockFlag.load());
		Slowers = Slowing.Slowers;
		CombinedSlowMult = Slowing.CombinedSlowMult;
		return *this;
	}

};