#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "RvoSimulator.h"
#include "RVOVector2.h"
#include "Traits/GridData.h"
#include "Avoidance.generated.h"
   
UENUM(BlueprintType)
enum class EAvoidMode : uint8
{
    RVO2 UMETA(DisplayName = "RVO2"),
    PBD UMETA(DisplayName = "PBD")
};

USTRUCT(BlueprintType, Category = "Avoidance")
struct BATTLEFRAME_API FAvoidance
{
    GENERATED_BODY()
 
public:

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "是否启用避障功能"))
    bool bEnable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "碰撞组", ClampMin = "0", ClampMax = "9"))
    int32 Group = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "不与这些组产生碰撞", ClampMin = "0", ClampMax = "9"))
    TArray<int32> IgnoreGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Trace Radius", ToolTip = "避障视野 = Collider.Radius * Scaled.Scale + TraceRadius", ClampMin = "0"))
    float TraceDist = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Avoid Radius Multiplier", ToolTip = "与其它Agent保持的距离 = Collider.Radius * Scaled.Scale * AvoidRadiusMultiplier", ClampMin = "0"))
    float AvoidDistMult = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "列入考量的邻居最大数量，近距优先", ClampMin = "1"))
    int32 MaxNeighbors = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "TimeHorizon_Agent", ToolTip = "提前躲避这个时间后会碰撞的单位", ClampMin = "0"))
    float RVO_TimeHorizon_Agent = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "TimeHorizon_Obstacle", ToolTip = "提前躲避这个时间后会碰撞的障碍物", ClampMin = "0"))
    float RVO_TimeHorizon_Obstacle = 0.1f;


    //-------------------------------------------------------------------------------

    float MaxSpeed = 0.f;
    std::vector<RVO::Line> OrcaLines;
    RVO::Vector2 DesiredVelocity = RVO::Vector2(0.0f, 0.0f);
    RVO::Vector2 AvoidingVelocity = RVO::Vector2(0.0f, 0.0f);

    FAvoidance(){}
};
