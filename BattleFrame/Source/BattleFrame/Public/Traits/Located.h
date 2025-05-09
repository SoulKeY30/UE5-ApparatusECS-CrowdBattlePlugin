#pragma once

#include "CoreMinimal.h"
#include "Located.generated.h"

USTRUCT(BlueprintType, Category = "Basic")
struct BATTLEFRAME_API FLocated
{
	GENERATED_BODY()

  public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	FVector PreLocation = FVector::ZeroVector; 

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	FVector InitialLocation = FVector::ZeroVector;

};
