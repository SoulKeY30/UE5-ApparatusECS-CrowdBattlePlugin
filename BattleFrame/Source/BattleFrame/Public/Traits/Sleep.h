#pragma once

#include "CoreMinimal.h"
#include "Sleep.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSleep
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bEnable = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float WakeRadius = 1000.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = 0, UIMax = 360))
	float WakeAngle = 60.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bWakeOnHit = true;



	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	//bool bCheckVisibility = true;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	//TEnumAsByte<EObjectTypeQuery> TraceObjectType;

};