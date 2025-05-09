#pragma once
 
#include "CoreMinimal.h"
#include "Attacking.generated.h"

UENUM(BlueprintType)
enum class EAttackState : uint8
{
	PreCast UMETA(DisplayName = "PreCast"),
	PostCast UMETA(DisplayName = "PostCast"),
	Cooling UMETA(DisplayName = "Cooling")
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FAttacking
{
	GENERATED_BODY()
 
  public:

	float Time = 0.0f;

	EAttackState State = EAttackState::PreCast;

};
