#pragma once
 
#include "CoreMinimal.h"
#include "FxRecord.generated.h"


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FFxRecord
{
	GENERATED_BODY()

public:

	/* �ӵ�ģ��*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FSubjectRecord FxRecord = FSubjectRecord();

};