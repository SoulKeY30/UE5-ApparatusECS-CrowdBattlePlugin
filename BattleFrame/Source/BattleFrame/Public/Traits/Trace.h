#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "BattleFrameStructs.h"
#include "Trace.generated.h"


UENUM(BlueprintType)
enum class ETraceMode : uint8
{
	TargetIsPlayer_0 UMETA(DisplayName = "IsPlayer_0", Tooltip = "索敌目标为玩家0"),
	SectorTraceByTraits UMETA(DisplayName = "ByTraits", Tooltip = "根据特征进行扇形索敌")
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSectorTraceShape
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "通用的扇形索敌参数", DisplayName = "Common"))
	FSectorTraceParams Common = FSectorTraceParams{ true };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "Sleep专用扇形索敌范围，可禁用，禁用后使用通用参数", DisplayName = "Sleep"))
	FSectorTraceParamsSpecific Sleep = FSectorTraceParamsSpecific{ false };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "Patrol专用扇形索敌范围，可禁用，禁用后使用通用参数", DisplayName = "Patrol"))
	FSectorTraceParamsSpecific Patrol = FSectorTraceParamsSpecific{ false };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "Chase专用扇形索敌范围，可禁用，禁用后使用通用参数", DisplayName = "Chase"))
	FSectorTraceParamsSpecific Chase = FSectorTraceParamsSpecific{ false };

};


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTrace
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用索敌功能"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "绘制Debug图形"))
	bool bDrawDebugShape = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "扇形索敌参数"))
	FSectorTraceShape SectorTrace = FSectorTraceShape();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "索敌冷却时间（秒）"))
	float CoolDown = 2;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "索敌模式 (TargetIsPlayer_0: 目标为玩家, SphereTraceByTraits: 根据特征索敌)"))
	ETraceMode Mode = ETraceMode::TargetIsPlayer_0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "包含具有这些特征的目标"))
	TArray<UScriptStruct*> IncludeTraits;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "排除具有这些特征的目标"))
	TArray<UScriptStruct*> ExcludeTraits;

};
