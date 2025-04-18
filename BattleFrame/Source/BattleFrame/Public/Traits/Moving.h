#pragma once
 
#include "CoreMinimal.h"
#include "Math/Vector.h"

#include "Moving.generated.h"
 
/**
 * The Moving command.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FMoving
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

	  UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "想移动的速度与方向"))
	  FVector DesiredVelocity = FVector::ZeroVector;

	  UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "实际移动的速度与方向"))
	  FVector CurrentVelocity = FVector::ZeroVector;

	  UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (Tooltip = ""))
	  bool bFalling = false;

	  UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (Tooltip = ""))
	  bool bLaunching = false;

	  UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (Tooltip = ""))
	  bool bPushedBack = false;

	  FVector LaunchForce = FVector::ZeroVector;
	  float LaunchTimer = 0;
	  float PushBackSpeedOverride = 0;
	  float FlyingHeight = 0;
	  float SpeedMult = 1;

	  FMoving() {};

	  FMoving(const FMoving& Moving)
	  {
	  	LockFlag.store(Moving.LockFlag.load());

		DesiredVelocity = Moving.DesiredVelocity;
		CurrentVelocity = Moving.CurrentVelocity;

		PushBackSpeedOverride = Moving.PushBackSpeedOverride;
	  	LaunchForce = Moving.LaunchForce;
		LaunchTimer = Moving.LaunchTimer;
		FlyingHeight = Moving.FlyingHeight;
		SpeedMult = Moving.SpeedMult;

		bFalling = Moving.bFalling;
		bLaunching = Moving.bLaunching;
		bPushedBack = Moving.bPushedBack;
	  }

	  FMoving& operator=(const FMoving& Moving)
	  {
	  	LockFlag.store(Moving.LockFlag.load());

		DesiredVelocity = Moving.DesiredVelocity;
		CurrentVelocity = Moving.CurrentVelocity;

		PushBackSpeedOverride = Moving.PushBackSpeedOverride;
		LaunchForce = Moving.LaunchForce;
		LaunchTimer = Moving.LaunchTimer;
		FlyingHeight = Moving.FlyingHeight;
		SpeedMult = Moving.SpeedMult;

		bFalling = Moving.bFalling;
		bLaunching = Moving.bLaunching;
		bPushedBack = Moving.bPushedBack;

	  	return *this;
	  }

};
