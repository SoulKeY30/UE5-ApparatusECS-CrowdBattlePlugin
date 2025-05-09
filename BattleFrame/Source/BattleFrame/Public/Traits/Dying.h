#pragma once
 
#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "Dying.generated.h"
 
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDying
{
	GENERATED_BODY()
 
  public:

	float Time = 0.0f;

	float Duration = 0.0f;

	FSubjectHandle Instigator = FSubjectHandle();

};
