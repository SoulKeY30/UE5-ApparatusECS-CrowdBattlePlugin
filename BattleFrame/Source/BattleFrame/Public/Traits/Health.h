#pragma once
 
#include "CoreMinimal.h"
#include <atomic>
#include "Health.generated.h"
 
/**
 * The health level of the character.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FHealth
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

	/**
	 * The current health value.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "当前生命值"))
	float Current = 100.f;

	/**
	 * The maximum health value.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "最大生命值"))
	float Maximum = 100.f;

	TQueue<float, EQueueMode::Mpsc> DamageToTake;
	TQueue<FSubjectHandle, EQueueMode::Mpsc> DamageInstigator;

	FHealth() {};

	FHealth(const FHealth& Health)
	{
		LockFlag.store(Health.LockFlag.load());
		Current = Health.Current;
		Maximum = Health.Maximum;
	}

	FHealth& operator=(const FHealth& Health)
	{
		LockFlag.store(Health.LockFlag.load());
		Current = Health.Current;
		Maximum = Health.Maximum;
		return *this;
	}
};
