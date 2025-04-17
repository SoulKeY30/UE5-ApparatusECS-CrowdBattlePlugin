#pragma once

#include "CoreMinimal.h"
#include "ActorSpawnConfig.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FActorSpawnConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	TSubclassOf<AActor> ActorClass = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	FTransform Transform = FTransform::Identity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	int32 Quantity = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	float Delay = 0;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = ""))
	//bool bAttached = false;

	//-----------------------------------------------

	float TimeLeft = 0;

	FSubjectHandle Owner;

	FTransform SubjectTrans;

};
