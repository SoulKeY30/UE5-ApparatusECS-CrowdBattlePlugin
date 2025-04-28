#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundBase.h"
#include "SubjectHandle.h"
#include "SoundConfig.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSoundConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	TSoftObjectPtr<USoundBase> Sound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	float Delay = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	FTransform Transform = FTransform::Identity;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = ""))
	//bool bAttached = false;

	//-----------------------------------------------

	float TimeLeft = 0;

	FSubjectHandle Owner = FSubjectHandle();

	FTransform SubjectTrans = FTransform::Identity;

};
