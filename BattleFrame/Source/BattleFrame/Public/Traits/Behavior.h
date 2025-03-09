#pragma once

#include "CoreMinimal.h"
#include "Behavior.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FBehavior
{
	GENERATED_BODY()

public:

	bool bAppear = false;

	bool bSleep = false;

	bool bPatrol = false;

	bool bChase = true;

	bool bAttack = true;

	bool bEscape = false;

	bool bHit = true;

	bool bDeath = true;

};