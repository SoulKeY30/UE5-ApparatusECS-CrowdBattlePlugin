#pragma once

#include "CoreMinimal.h"
#include "RVOVector2.h"
#include "SubjectHandle.h"
#include "BoxObstacle.generated.h"

USTRUCT(BlueprintType, Category = "RVO")
struct BATTLEFRAME_API FBoxObstacle
{
    GENERATED_BODY()

public:

    FSubjectHandle nextObstacle_ = FSubjectHandle();
    FSubjectHandle prevObstacle_ = FSubjectHandle();

    RVO::Vector2 point_;
    RVO::Vector2 prePoint_;
    RVO::Vector2 nextPoint_;
    RVO::Vector2 nextNextPoint_;
    RVO::Vector2 unitDir_;

    float pointZ_;
    float height_;

    bool isConvex_ = true;
    bool bStatic = false;
    bool bRegistered = false;
    bool bExcluded = false;

};