#pragma once
 
#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "DmgInstigator.generated.h"
 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDmgInstigator
{
	GENERATED_BODY()
 
  public:

	  FSubjectHandle DmgInstigator = FSubjectHandle();
};
