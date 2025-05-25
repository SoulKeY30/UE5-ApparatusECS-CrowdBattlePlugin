#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "RvoSimulator.h"
#include "RVOVector2.h"
#include "Avoiding.generated.h"


// these values are cached for cpu cache optimizaiton
USTRUCT(BlueprintType, Category = "Avoidance")
struct BATTLEFRAME_API FAvoiding
{
    GENERATED_BODY()

public:

    bool bCanAvoid = true;
    float Radius = 100.0f;
    RVO::Vector2 CurrentVelocity = RVO::Vector2(0.0f, 0.0f);
    RVO::Vector2 Position = RVO::Vector2(0.0f, 0.0f);
};
