#pragma once
 
#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "Traits/Damage.h"
#include "TemporalDamager.generated.h"
 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTemporalDamager
{
	GENERATED_BODY()
 
  public:

	  FSubjectHandle TemporalDamageInstigator = FSubjectHandle();
	  FSubjectHandle TemporalDamageTarget = FSubjectHandle();

	  float TemporalDamageTimeout = 0.5f;
	  float TemporalDmgInterval = 0.5f;  // 伤害间隔

	  float RemainingTemporalDamage = 0.f;
	  float TotalTemporalDamage = 0.f;

	  int32 CurrentSegment = 0;          // 当前伤害段数
	  int32 TemporalDmgSegment = 4;      // 总伤害段数

	  EDmgType DmgType = EDmgType::Normal;

	  bool bJustSpawned = true;

};
