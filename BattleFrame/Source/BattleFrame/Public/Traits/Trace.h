#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "Trace.generated.h"

class UNeighborGridComponent;

UENUM(BlueprintType)
enum class ETraceMode : uint8
{
	TargetIsPlayer_0 UMETA(DisplayName = "IsPlayer_0", Tooltip = "索敌目标为玩家0"),
	SphereTraceByTraits UMETA(DisplayName = "ByTraits", Tooltip = "根据特征进行球形索敌")
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTrace
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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用索敌功能"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "索敌模式 (TargetIsPlayer_0: 目标为玩家, CustomTarget: 自定义目标, SphereTraceByTraits: 根据特征索敌)"))
	ETraceMode Mode = ETraceMode::TargetIsPlayer_0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "包含的索敌特征列表 (仅索敌具有这些特征的目标)"))
	TArray<UScriptStruct*> IncludeTraits;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "排除的索敌特征列表 (忽略具有这些特征的目标)"))
	TArray<UScriptStruct*> ExcludeTraits;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "索敌冷却时间（秒）"))
	float CoolDown = 2;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "索敌范围（单位：厘米）"))
	float Radius = 300;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = 0, UIMax = 360))
	float Angle = 360.f;

	//----------------------------------------

	FSubjectHandle TraceResult = FSubjectHandle();

	UNeighborGridComponent* NeighborGrid = nullptr;

	float TimeLeft = 0.f;

	//----------------------------------------

	FTrace() {};

	FTrace(const FTrace& Trace)
	{
		LockFlag.store(Trace.LockFlag.load());

		NeighborGrid = Trace.NeighborGrid;
		TraceResult = Trace.TraceResult;
		bEnable = Trace.bEnable;
		Mode = Trace.Mode;
		IncludeTraits = Trace.IncludeTraits;
		ExcludeTraits = Trace.ExcludeTraits;
		CoolDown = Trace.CoolDown;
		Radius = Trace.Radius;
		Angle = Trace.Angle;
	}

	FTrace& operator=(const FTrace& Trace)
	{
		LockFlag.store(Trace.LockFlag.load());

		NeighborGrid = Trace.NeighborGrid;
		TraceResult = Trace.TraceResult;
		bEnable = Trace.bEnable;
		Mode = Trace.Mode;
		IncludeTraits = Trace.IncludeTraits;
		ExcludeTraits = Trace.ExcludeTraits;
		CoolDown = Trace.CoolDown;
		Radius = Trace.Radius;
		Angle = Trace.Angle;

		return *this;
	}

};
