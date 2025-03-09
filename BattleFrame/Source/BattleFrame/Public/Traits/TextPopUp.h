#pragma once

#include "CoreMinimal.h"
#include "TextPopUp.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTextPopUp
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用文本弹出效果"))
	bool Enable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "白色文本显示的百分比阈值（低于此值时显示白色文本）"))
	float WhiteTextBelowPercent = 0.333f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "橙色文本显示的百分比阈值（低于此值时显示黄色文本）"))
	float OrangeTextAbovePercent = 0.667f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "文本大小"))
	float TextScale = 300;
};
