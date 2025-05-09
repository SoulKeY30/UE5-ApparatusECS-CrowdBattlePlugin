/*
 * APPARATIST
 * Created: 2023-02-02 16:26:26
 * Author: Vladislav Dmitrievich Turbanov (vladislav@turbanov.ru)
 * Community forums: https://talk.turbanov.ru
 * Copyright 2019 - 2023, SP Vladislav Dmitrievich Turbanov
 * Made in Russia, Moscow City, Chekhov City
 */

 /*
  * BattleFrame
  * Refactor: 2025
  * Author: Leroy Works
  */

#pragma once

#include "CoreMinimal.h"
#include "Math/Rotator.h"
#include "Rotated.generated.h"


USTRUCT(BlueprintType, Category = "Basic")
struct BATTLEFRAME_API FRotated
{
	GENERATED_BODY()

  public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	FQuat Rotation = FQuat{ FQuat::Identity };

};
