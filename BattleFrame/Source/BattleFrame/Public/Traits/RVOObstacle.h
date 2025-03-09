#pragma once

#include "CoreMinimal.h"
#include "Vector2.h"
#include "RVOObstacle.generated.h"

USTRUCT(BlueprintType, Category = "RVO")
struct BATTLEFRAME_API FRVOObstacle
{
    GENERATED_BODY()

public:
    bool isConvex_ = true;

    FSubjectHandle nextObstacle_;

    RVO::Vector2 point_;

    FSubjectHandle prevObstacle_;

    RVO::Vector2 unitDir_;

    FVector point3d_;

    float height_;
};