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

	  UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (Tooltip = ""))
	  FVector DesiredVelocity = FVector::ZeroVector;

	  UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (Tooltip = ""))
	  FVector CurrentVelocity = FVector::ZeroVector;

	  UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (Tooltip = ""))
	  bool bFalling = false;

	  UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (Tooltip = ""))
	  bool bLaunching = false;

	  UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (Tooltip = ""))
	  bool bPushedBack = false;

	  FVector LaunchVelSum = FVector::ZeroVector;
	  float PushBackSpeedOverride = 0;
	  float FlyingHeight = 0;
	  float MoveSpeedMult = 1;
	  float TurnSpeedMult = 1;
	  float CurrentAngularVelocity = 0;
	  FVector Goal = FVector::ZeroVector;

	  // 速度历史记录
	  TArray<FVector,TInlineAllocator<5>> HistoryVelocity;
	  FVector AverageVelocity = FVector::ZeroVector;
	  FVector CachedSumVelocity = FVector::ZeroVector;
	  int32 CurrentIndex = 0;
	  float TimeLeft = 0.f;
	  bool bShouldInit = true;
	  bool bBufferFilled = false;

	  FMoving() {};

	  // 更新速度记录
	  FORCEINLINE void Initialize()
	  {
		  HistoryVelocity.Init(FVector::ZeroVector, 5);
		  bShouldInit = false;
	  }

	  FORCEINLINE void UpdateVelocityHistory(const FVector& NewVelocity)
	  {
		  // 减去即将被覆盖的值
		  CachedSumVelocity -= HistoryVelocity[CurrentIndex];

		  // 记录新值并更新缓存
		  HistoryVelocity[CurrentIndex] = NewVelocity;
		  CachedSumVelocity += NewVelocity;

		  // 更新环形索引
		  CurrentIndex = (CurrentIndex + 1) % 5;

		  // 标记缓冲区是否已填满
		  if (!bBufferFilled && CurrentIndex == 0)
		  {
			  bBufferFilled = true;
		  }

		  // 计算当前平均速度
		  const int32 ValidFrames = bBufferFilled ? 5 : CurrentIndex;
		  AverageVelocity = (ValidFrames > 0) ? (CachedSumVelocity / ValidFrames) : FVector::ZeroVector;

		  TimeLeft = 0.1f;
	  }

	  FMoving(const FMoving& Moving)
	  {
	  	LockFlag.store(Moving.LockFlag.load());

		DesiredVelocity = Moving.DesiredVelocity;
		CurrentVelocity = Moving.CurrentVelocity;
		bFalling = Moving.bFalling;
		bLaunching = Moving.bLaunching;
		bPushedBack = Moving.bPushedBack;
		LaunchVelSum = Moving.LaunchVelSum;
		PushBackSpeedOverride = Moving.PushBackSpeedOverride;
		FlyingHeight = Moving.FlyingHeight;
		MoveSpeedMult = Moving.MoveSpeedMult;
		TurnSpeedMult = Moving.TurnSpeedMult;
		CurrentAngularVelocity = Moving.CurrentAngularVelocity;
		Goal = Moving.Goal;

		HistoryVelocity = Moving.HistoryVelocity;
		AverageVelocity = Moving.AverageVelocity;
		CachedSumVelocity = Moving.CachedSumVelocity;
		CurrentIndex = Moving.CurrentIndex;
		TimeLeft = Moving.TimeLeft;
		bShouldInit = Moving.bShouldInit;
		bBufferFilled = Moving.bBufferFilled;
	  }

	  FMoving& operator=(const FMoving& Moving)
	  {
	  	LockFlag.store(Moving.LockFlag.load());

		DesiredVelocity = Moving.DesiredVelocity;
		CurrentVelocity = Moving.CurrentVelocity;
		bFalling = Moving.bFalling;
		bLaunching = Moving.bLaunching;
		bPushedBack = Moving.bPushedBack;
		LaunchVelSum = Moving.LaunchVelSum;
		PushBackSpeedOverride = Moving.PushBackSpeedOverride;
		FlyingHeight = Moving.FlyingHeight;
		MoveSpeedMult = Moving.MoveSpeedMult;
		TurnSpeedMult = Moving.TurnSpeedMult;
		CurrentAngularVelocity = Moving.CurrentAngularVelocity;
		Goal = Moving.Goal;

		HistoryVelocity = Moving.HistoryVelocity;
		AverageVelocity = Moving.AverageVelocity;
		CachedSumVelocity = Moving.CachedSumVelocity;
		CurrentIndex = Moving.CurrentIndex;
		TimeLeft = Moving.TimeLeft;
		bShouldInit = Moving.bShouldInit;
		bBufferFilled = Moving.bBufferFilled;

	  	return *this;
	  }
};
