#pragma once

#include "CoreMinimal.h"
#include "Hit.generated.h"

/**
 * The state of being hit by a projectile.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FHit
{
	GENERATED_BODY()

public:

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用被击中效果"))
	//bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用被击中时的发光效果"))
	bool bCanGlow = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "被击中时的挤压/拉伸强度"))
	float SqueezeSquashStr = 0.5f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否播放被击中特效"))
	bool bCanSpawnFx = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否播放被击中音效"))
	bool bCanPlaySound = true;
};
