
#pragma once

#include "CoreMinimal.h"

#include "Scaled.generated.h"


/**
 * A general scaling provider.
 */
USTRUCT(BlueprintType, Category = "Basic")
struct BATTLEFRAME_API FScaled
{
	GENERATED_BODY()

  public:

	/// The scaling factors to apply.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	FVector Factors = FVector::OneVector;

	FVector renderFactors = FVector::OneVector;

	/// Default constructor.
	FScaled() {}

	/// Construct with a factor.
	FORCEINLINE
	FScaled(const float InFactor)
	  : Factors(InFactor)
	{}

	/// Construct with factors vector.
	FORCEINLINE
	FScaled(const FVector InFactors)
	  : Factors(InFactors)
	{}

	/// Construct copying from a transform.
	FORCEINLINE
	FScaled(const FTransform& InTransform)
	  : Factors(InTransform.GetScale3D())
	{}
};
