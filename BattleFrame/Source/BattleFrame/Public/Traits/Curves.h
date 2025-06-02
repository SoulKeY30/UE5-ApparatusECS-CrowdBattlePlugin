#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "Curves.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FCurves
{
    GENERATED_BODY()

public:

    FCurves()
    {
        InitializeCurve(DissolveIn, { {0.0f, 0.0f}, {1.0f, 1.0f} });
        InitializeCurve(DissolveOut, { {0.0f, 1.0f}, {1.0f, 0.0f} });
        InitializeCurve(HitEmission, { {0.0f, 0.0f}, {0.1f, 1.0f}, {0.5f, 0.0f} });
        InitializeCurve(HitJiggle, { {0.0f, 1.0f}, {0.1f, 1.75f}, {0.28f, 0.78f}, {0.4f, 1.12f}, {0.5f, 1.0f} });
    }

private:

    void InitializeCurve(FRuntimeFloatCurve& Curve, const TArray<TPair<float, float>>& Keyframes)
    {
        Curve.GetRichCurve()->Reset();

        for (const auto& Keyframe : Keyframes)
        {
            FKeyHandle KeyHandle = Curve.GetRichCurve()->AddKey(Keyframe.Key, Keyframe.Value);
            Curve.GetRichCurve()->SetKeyInterpMode(KeyHandle, RCIM_Cubic);
            Curve.GetRichCurve()->SetKeyTangentMode(KeyHandle, RCTM_Auto);
        }
    }

public:

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Curves, meta = (Tooltip = "淡入效果的曲线 (控制溶解效果的过渡)"))
    FRuntimeFloatCurve DissolveIn;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Curves, meta = (Tooltip = "淡出效果的曲线 (控制溶解效果的过渡)"))
    FRuntimeFloatCurve DissolveOut;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Curves, meta = (Tooltip = "受击发光效果的曲线 (控制发光强度的变化)"))
    FRuntimeFloatCurve HitEmission;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Curves, meta = (Tooltip = "受击挤压/拉伸效果的曲线 (控制模型的形变)"))
    FRuntimeFloatCurve HitJiggle;
};
