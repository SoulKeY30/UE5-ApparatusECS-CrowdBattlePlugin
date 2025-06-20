#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include <Containers/Queue.h>
#include "Health.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FHealth
{
	GENERATED_BODY()

public:

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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "锁定生命值"))
	bool bLockHealth = false;

	TQueue<float, EQueueMode::Mpsc> DamageToTake;
	TQueue<FSubjectHandle, EQueueMode::Mpsc> DamageInstigator;
	TQueue<FVector, EQueueMode::Mpsc> HitDirection;

	// 默认构造函数
	FHealth() = default;

	FHealth(const FHealth& Health)
	{
		Current = Health.Current;
		Maximum = Health.Maximum;
		bLockHealth = Health.bLockHealth;
	}

	FHealth& operator=(const FHealth& Health)
	{
		Current = Health.Current;
		Maximum = Health.Maximum;
		bLockHealth = Health.bLockHealth;
		return *this;
	}

};
