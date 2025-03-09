#pragma once
 
#include "CoreMinimal.h"
#include "SolidSubjectHandle.h"
#include "TemporalDamager.generated.h"
 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTemporalDamager
{
	GENERATED_BODY()
 
  public:

	  FSolidSubjectHandle TemporalDamageInstigator = FSolidSubjectHandle();
	  FSolidSubjectHandle TemporalDamageTarget = FSolidSubjectHandle();
	  float TemporalDamageTimeout = 0.5f;
	  float TotalTemporalDamage = 0.f;
	  float RemainingTemporalDamage = 0.f;
};
