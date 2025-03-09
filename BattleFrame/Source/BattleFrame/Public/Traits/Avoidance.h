#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "Traits/Avoiding.h"
#include "RvoSimulator.h"
#include "Vector2.h"

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

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Avoidance", meta = (ToolTip = "是否启用避障功能"))
    bool bEnable = true;

    // Initialize variables with reasonable default values
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance", meta = (ToolTip = "考虑其他代理为邻居的距离"))
    float NeighborDist = 150.0f;  // Distance within which the agent will consider other agents as neighbors

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance", meta = (ToolTip = "最大邻居数量"))
    int32 MaxNeighbors = 36;  // Maximum number of neighbors to consider for Collider avoidance 4*9

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance", meta = (ToolTip = "最大速度"))
    float SpeedLimit = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance", meta = (ToolTip = "RVO时间范围"))
    float RVO_TimeHorizon = 1.0f;  // Time horizon over which agent takes future agent positions into account

    //-------------------------------------------------------------------------------

    float Radius = 100.0f;  // Radius of the agent for Collider calculations
    float MaxSpeed = 0.f;  // Maximum speed of the agent
    float TimeHorizonObst = 1.0f;  // Time horizon over which agent takes future obstacle positions into account
    std::vector<RVO::Line> OrcaLines;  // ORCA lines for Collider avoidance computation
    RVO::Vector2 Position = RVO::Vector2(0.0f, 0.0f);  // Current position of the agent
    RVO::Vector2 CurrentVelocity = RVO::Vector2(0.0f, 0.0f);  // Current velocity of the agent, initially at rest
    RVO::Vector2 DesiredVelocity = RVO::Vector2(0.0f, 0.0f);  // Preferred velocity of the agent towards its goal
    RVO::Vector2 AvoidingVelocity = RVO::Vector2(0.0f, 0.0f);  // New velocity calculated by RVO algorithm based on current scenario 

    //-------------------------------------------------------------------------------

    TSet<FAvoiding> SubjectNeighbors;
    TArray<FAvoiding> ObstacleNeighbors;

    FFilter SubjectFilter;

};
