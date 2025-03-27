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

<<<<<<< HEAD
	  UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "想移动的速度与方向"))
	  FVector DesiredVelocity = FVector::ZeroVector;

	  UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "实际移动的速度与方向"))
	  FVector CurrentVelocity = FVector::ZeroVector;
=======
	  FVector KnockBackForce = FVector::ZeroVector;
	  FVector LaunchForce = FVector::ZeroVector;
	  float PushBackSpeedOverride = 0;
	  FVector Direction = FVector::ZeroVector;
	  float Speed = 0.f;
	  FVector Velocity = FVector::ZeroVector;
>>>>>>> parent of 0f9a801 (Beta.2)

	  bool bFalling = false;
	  bool bLaunching = false;
	  bool bKnockedBack = false;
	  bool bPushedBack = false;

<<<<<<< HEAD
	  FVector LaunchForce = FVector::ZeroVector;
	  float LaunchTimer = 0;
	  float PushBackSpeedOverride = 0;
	  float FlyingHeight = 0;
	  float SpeedMult = 1;

=======
>>>>>>> parent of 0f9a801 (Beta.2)
	  FMoving() {};

	  FMoving(const FMoving& Moving)
	  {
	  	LockFlag.store(Moving.LockFlag.load());

	  	Speed = Moving.Speed;
		PushBackSpeedOverride = Moving.PushBackSpeedOverride;
	  	KnockBackForce = Moving.KnockBackForce;
	  	LaunchForce = Moving.LaunchForce;
<<<<<<< HEAD
		LaunchTimer = Moving.LaunchTimer;
		FlyingHeight = Moving.FlyingHeight;
		SpeedMult = Moving.SpeedMult;
=======
	  	Direction = Moving.Direction;
	  	Velocity = Moving.Velocity;
>>>>>>> parent of 0f9a801 (Beta.2)

		bFalling = Moving.bFalling;
		bLaunching = Moving.bLaunching;
		bKnockedBack = Moving.bKnockedBack;
		bPushedBack = Moving.bPushedBack;
	  }

	  FMoving& operator=(const FMoving& Moving)
	  {
	  	LockFlag.store(Moving.LockFlag.load());

		Speed = Moving.Speed;
		PushBackSpeedOverride = Moving.PushBackSpeedOverride;
		KnockBackForce = Moving.KnockBackForce;
		LaunchForce = Moving.LaunchForce;
<<<<<<< HEAD
		LaunchTimer = Moving.LaunchTimer;
		FlyingHeight = Moving.FlyingHeight;
		SpeedMult = Moving.SpeedMult;
=======
		Direction = Moving.Direction;
		Velocity = Moving.Velocity;
>>>>>>> parent of 0f9a801 (Beta.2)

		bFalling = Moving.bFalling;
		bLaunching = Moving.bLaunching;
		bKnockedBack = Moving.bKnockedBack;
		bPushedBack = Moving.bPushedBack;

	  	return *this;
	  }

};
