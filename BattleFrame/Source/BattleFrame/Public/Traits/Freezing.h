#pragma once
 
#include "CoreMinimal.h"
#include "Freezing.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FFreezing
{
	GENERATED_BODY()

public:

	// ����ʱ��
	float SlowTimeout = 4.f;
	float SlowStr = 1.f;
	float OriginalDecoupleProportion = 1.f;
	//float CurrentSlowStr = 0.f;

};