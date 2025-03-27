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

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "是否启用避障功能"))
    bool bEnable = true;

<<<<<<< HEAD
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "碰撞组", ClampMin = "0", ClampMax = "9"))
    int32 Group = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "不与这些组产生碰撞", ClampMin = "0", ClampMax = "9"))
    TArray<int32> IgnoreGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "视野距离", ClampMin = "0"))
    float NeighborDist = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "最大邻居数量", ClampMin = "1"))
    int32 MaxNeighbors = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "提前躲避这个时间后会碰撞的单位", ClampMin = "0"))
    float RVO_TimeHorizon_Agent = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "提前躲避这个时间后会碰撞的障碍物", ClampMin = "0"))
    float RVO_TimeHorizon_Obstacle = 0.1f;

    //-------------------------------------------------------------------------------

    float Radius = 100.0f;
    float MaxSpeed = 0.f;
    
    std::vector<RVO::Line> OrcaLines;
    RVO::Vector2 Position = RVO::Vector2(0.0f, 0.0f);
    RVO::Vector2 CurrentVelocity = RVO::Vector2(0.0f, 0.0f);
    RVO::Vector2 DesiredVelocity = RVO::Vector2(0.0f, 0.0f);
    RVO::Vector2 AvoidingVelocity = RVO::Vector2(0.0f, 0.0f);
=======
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
>>>>>>> parent of 0f9a801 (Beta.2)

    TSet<FAvoiding> SubjectNeighbors;
    TArray<FAvoiding> ObstacleNeighbors;

    //FFilter SubjectFilter;

<<<<<<< HEAD
    //TArray<FAvoiding> SubjectNeighbors;
    //TArray<FAvoiding> SphereObstacleNeighbors;
    //TArray<FAvoiding> BoxObstacleNeighbors;

=======
>>>>>>> parent of 0f9a801 (Beta.2)
};
