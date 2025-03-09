#pragma once

#include "CoreMinimal.h"
#include "SubType.h"
#include "Appear.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FAppear
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用兵种出生逻辑"))
    bool bEnable = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "兵种出生前的延迟时间（秒）"))
    float Delay = 0.f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "兵种出生的总持续时间（秒）"))
    float Duration = 1.f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否启用淡入效果（溶解）"))
    bool bCanDissolveIn = false;

    //UPROPERTY(BlueprintReadWrite, EditAnywhere)//WIP
    bool bCanScaleIn = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否播放出生动画"))
    bool bCanPlayAnim = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否生成出生时的贴花"))
    bool bCanSpawnDecal = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否播放出生特效"))
    bool bCanSpawnFx = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "是否播放出生音效"))
    bool bCanPlaySound = true;

    bool bAppearStarted = false;
};
