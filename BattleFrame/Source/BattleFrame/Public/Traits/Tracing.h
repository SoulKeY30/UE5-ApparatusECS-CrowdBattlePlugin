#pragma once

#include "CoreMinimal.h"
//#include "SubjectHandle.h"
#include "Tracing.generated.h"


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTracing
{
	GENERATED_BODY()

public:

	bool bShouldTrace = false;
	float TimeLeft = 0;

};
