#pragma once

#include "CoreMinimal.h"
#include "Scaled.generated.h"


USTRUCT(BlueprintType, Category = "Basic")
struct BATTLEFRAME_API FScaled
{
	GENERATED_BODY()

  public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	FVector Factors = FVector::OneVector;

	FVector renderFactors = FVector::OneVector;

};
