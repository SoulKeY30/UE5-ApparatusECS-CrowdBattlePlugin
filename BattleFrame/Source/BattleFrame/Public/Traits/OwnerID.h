#pragma once
 
#include "CoreMinimal.h"
#include "OwnerID.generated.h"
 
/**
 * The movement speed factor.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FOwnerID
{
	GENERATED_BODY()
 
  public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 ID = 0;
};
