#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "TemporalDamaging.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTemporalDamaging
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

	TSet<FSubjectHandle> TemporalDamagers;

	FTemporalDamaging() {};

	FTemporalDamaging(const FTemporalDamaging& TemporalDamaging)
	{
		LockFlag.store(TemporalDamaging.LockFlag.load());
		TemporalDamagers = TemporalDamaging.TemporalDamagers;
	}

	FTemporalDamaging& operator=(const FTemporalDamaging& TemporalDamaging)
	{
		LockFlag.store(TemporalDamaging.LockFlag.load());
		TemporalDamagers = TemporalDamaging.TemporalDamagers;
		return *this;
	}
};