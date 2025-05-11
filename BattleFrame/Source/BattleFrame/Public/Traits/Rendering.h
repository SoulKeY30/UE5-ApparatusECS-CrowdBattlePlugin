
#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "Rendering.generated.h"


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FRendering
{
    GENERATED_BODY()

public:
    int32 InstanceId = -1;

    FSubjectHandle Renderer = FSubjectHandle();

};
