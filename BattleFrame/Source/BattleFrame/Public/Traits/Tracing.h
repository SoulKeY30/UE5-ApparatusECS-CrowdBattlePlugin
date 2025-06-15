#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "Tracing.generated.h"

class UNeighborGridComponent;

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTracing
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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "索敌结果"))
	FSubjectHandle TraceResult = FSubjectHandle();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "位于的邻居网格"))
	UNeighborGridComponent* NeighborGrid = nullptr;

	float TimeLeft = 0.f;


	FTracing() {};

	FTracing(const FTracing& Tracing)
	{
		LockFlag.store(Tracing.LockFlag.load());

		NeighborGrid = Tracing.NeighborGrid;
		TraceResult = Tracing.TraceResult;
		TimeLeft = Tracing.TimeLeft;
	}

	FTracing& operator=(const FTracing& Tracing)
	{
		LockFlag.store(Tracing.LockFlag.load());

		NeighborGrid = Tracing.NeighborGrid;
		TraceResult = Tracing.TraceResult;
		TimeLeft = Tracing.TimeLeft;

		return *this;
	}
};
