#pragma once

#include "CoreMinimal.h"
#include "Chase.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FChase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "与攻击目标距离低于该值时停止移动"))
	float AcceptanceRadius = 100.f;

};
