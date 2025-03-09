#pragma once
 
#include "CoreMinimal.h"
#include "TemporalDamaging.generated.h"
 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTemporalDamaging
{
	GENERATED_BODY()
 
  public:

	  FSubjectHandle TemporalDamageInstigator = FSubjectHandle();
	  FSubjectHandle TemporalDamageTarget = FSubjectHandle();
	  float TemporalDamageTimeout = 0.5f;
	  float TotalTemporalDamage = 0.f;
	  float RemainingTemporalDamage = 0.f;
	  bool firstRound = true;
};
