#pragma once
 
#include "CoreMinimal.h"
#include "Agent.generated.h"
 
/**
 * The main enemy trait.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FAgent
{
	GENERATED_BODY()
 
  public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Score = 1;
};
