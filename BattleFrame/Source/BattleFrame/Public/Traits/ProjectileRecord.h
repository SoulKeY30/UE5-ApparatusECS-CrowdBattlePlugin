#pragma once
 
#include "CoreMinimal.h"
#include "SubjectRecord.h"
#include "ProjectileRecord.generated.h"


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FProjectileRecord
{
	GENERATED_BODY()

public:

	/* ×Óµ¯Ä£°å*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FSubjectRecord ProjectileRecord = FSubjectRecord();
};