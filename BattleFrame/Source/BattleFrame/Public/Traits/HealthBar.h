#pragma once

#include "CoreMinimal.h"
#include "HealthBar.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FHealthBar
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否显示血条"))
	bool bShowHealthBar = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "血量满时是否隐藏血条"))
	bool HideOnFullHealth = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "血量为空时是否隐藏血条"))
	bool HideOnEmptyHealth = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否使用插值平滑血条变化"))
	bool UseInterpolation = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "血条插值变化的速度"))
	float InterpSpeed = 0.5f;

	float TargetRatio = 1.f;
	float CurrentRatio = 1.f;
	float Opacity = 0.f;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "血条的缩放比例"))
	//FVector Scale = FVector(1, 1, 1);

};
