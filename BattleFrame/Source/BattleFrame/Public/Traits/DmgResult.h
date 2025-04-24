#pragma once

#include "CoreMinimal.h"
#include "DmgResult.generated.h" 

USTRUCT(BlueprintType) struct FDmgResult
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FSubjectHandle DamagedSubject = FSubjectHandle();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool IsCritical = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool IsKill = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float DmgDealt = 0;
};
