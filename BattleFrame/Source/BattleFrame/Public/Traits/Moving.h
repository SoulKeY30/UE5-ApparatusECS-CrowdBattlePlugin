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

	  FVector KnockBackForce = FVector::ZeroVector;
	  FVector LaunchForce = FVector::ZeroVector;
	  float PushBackSpeedOverride = 0;
	  FVector Direction = FVector::ZeroVector;
	  float Speed = 0.f;
	  FVector Velocity = FVector::ZeroVector;

	  bool bFalling = false;
	  bool bLaunching = false;
	  bool bKnockedBack = false;
	  bool bPushedBack = false;

	  FMoving() {};

	  FMoving(const FMoving& Moving)
	  {
	  	LockFlag.store(Moving.LockFlag.load());

	  	Speed = Moving.Speed;
		PushBackSpeedOverride = Moving.PushBackSpeedOverride;
	  	KnockBackForce = Moving.KnockBackForce;
	  	LaunchForce = Moving.LaunchForce;
	  	Direction = Moving.Direction;
	  	Velocity = Moving.Velocity;

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
		Direction = Moving.Direction;
		Velocity = Moving.Velocity;

		bFalling = Moving.bFalling;
		bLaunching = Moving.bLaunching;
		bKnockedBack = Moving.bKnockedBack;
		bPushedBack = Moving.bPushedBack;

	  	return *this;
	  }

};
