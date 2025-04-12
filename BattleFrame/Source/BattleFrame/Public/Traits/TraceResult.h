#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "TraceResult.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTraceResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FSubjectHandle Subject;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector Location = FVector::ZeroVector;

    float CachedDistSq = -1.0f;
};
