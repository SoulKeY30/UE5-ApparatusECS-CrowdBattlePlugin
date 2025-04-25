#pragma once

#include "CoreMinimal.h"
#include "DmgResult.generated.h" 

USTRUCT(BlueprintType) struct FDmgResult
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FSubjectHandle DamagedSubject = FSubjectHandle();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FSubjectHandle InstigatorSubject = FSubjectHandle();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool IsCritical = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool IsKill = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float DmgDealt = 0;
};
