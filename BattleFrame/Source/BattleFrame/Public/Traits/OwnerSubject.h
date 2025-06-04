#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "OwnerSubject.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FOwnerSubject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "生成该Actor的Agent"))
	FSubjectHandle Owner = FSubjectHandle();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ToolTip = "管理该Actor的Subject"))
	FSubjectHandle Host = FSubjectHandle();

};