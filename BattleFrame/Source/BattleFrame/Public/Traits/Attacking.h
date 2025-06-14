#pragma once
 
#include "CoreMinimal.h"
#include "BattleFrameEnums.h"
#include "Attacking.generated.h"



USTRUCT(BlueprintType)
struct BATTLEFRAME_API FAttacking
{
	GENERATED_BODY()
 
  public:

	float Time = 0.0f;

	EAttackState State = EAttackState::Aim;

};
