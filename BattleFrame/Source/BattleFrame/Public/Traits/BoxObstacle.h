#pragma once

#include "CoreMinimal.h"
#include "RVOVector2.h"
#include "BoxObstacle.generated.h"

USTRUCT(BlueprintType, Category = "RVO")
struct BATTLEFRAME_API FBoxObstacle
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

    bool bStatic = false;

    bool bRegistered = false;

};