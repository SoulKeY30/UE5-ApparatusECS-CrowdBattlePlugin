#pragma once

#include "CoreMinimal.h"
#include "DmgResult.generated.h" 

USTRUCT(BlueprintType) struct FDmgResult
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FSubjectHandle> DamagedSubjects;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<bool> IsCritical;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<bool> IsKill;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<float> DmgDealt;
};
