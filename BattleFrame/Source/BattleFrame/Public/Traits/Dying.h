#pragma once
 
#include "CoreMinimal.h"
#include "Dying.generated.h"
 
/**
 * The state of dying.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDying
{
	GENERATED_BODY()
 
  public:

	/**
	 * The current time of the dying process in seconds.
	 */
	float Time = 0.0f;

	float Duration = 0.0f;

	FSubjectHandle Instigator;

};
