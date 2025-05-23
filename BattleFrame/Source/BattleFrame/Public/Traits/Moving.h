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

	  FVector LaunchVelSum = FVector::ZeroVector;
	  float PushBackSpeedOverride = 0;
	  float FlyingHeight = 0;
	  float SpeedMult = 1;

	  // 速度历史记录
	  TArray<FVector,TInlineAllocator<30>> VelHistory;
	  FVector VelAverage = FVector::ZeroVector;
	  FVector VelCachedSum = FVector::ZeroVector;
	  int32 CurrentIndex = 0;
	  float TimeLeft = 0.f;
	  bool bShouldInit = true;
	  bool bBufferFilled = false;

	  FMoving() {};

	  // 初始化方法
	  void Initialize()
	  {
		  VelHistory.Init(FVector::ZeroVector, 30);
		  bShouldInit = false;
	  }

	  // 更新速度记录
	  void UpdateVelocityHistory(const FVector& NewVelocity)
	  {
		  // 减去即将被覆盖的值
		  VelCachedSum -= VelHistory[CurrentIndex];

		  // 记录新值并更新缓存
		  VelHistory[CurrentIndex] = NewVelocity;
		  VelCachedSum += NewVelocity;

		  // 更新环形索引
		  CurrentIndex = (CurrentIndex + 1) % 30;

		  // 标记缓冲区是否已填满
		  if (!bBufferFilled && CurrentIndex == 0)
		  {
			  bBufferFilled = true;
		  }

		  // 计算当前平均速度
		  const int32 ValidFrames = bBufferFilled ? 30 : CurrentIndex;
		  VelAverage = (ValidFrames > 0) ? (VelCachedSum / ValidFrames) : FVector::ZeroVector;

		  TimeLeft = 0.0333f;
	  }

	  FMoving(const FMoving& Moving)
	  {
	  	LockFlag.store(Moving.LockFlag.load());
		bLaunching = Moving.bLaunching;
	  	LaunchVelSum = Moving.LaunchVelSum;
		FlyingHeight = Moving.FlyingHeight;
	  }

	  FMoving& operator=(const FMoving& Moving)
	  {
	  	LockFlag.store(Moving.LockFlag.load());
		bLaunching = Moving.bLaunching;
		LaunchVelSum = Moving.LaunchVelSum;
		FlyingHeight = Moving.FlyingHeight;
	  	return *this;
	  }

};
