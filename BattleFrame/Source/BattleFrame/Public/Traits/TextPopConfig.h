#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "TextPopConfig.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTextPopConfig
{
    GENERATED_BODY()

public:
    // 默认构造函数
    FTextPopConfig()
        : Owner(FSubjectHandle())
        , Value(0)
        , Style(0)
        , Scale(1)
        , Radius(0)
        , Location(FVector::ZeroVector)
    {
    }

    // 值构造函数
    FTextPopConfig(const FSubjectHandle& InOwner, float InValue, float InStyle,
        float InScale, float InRadius, const FVector& InLocation)
        : Owner(InOwner)
        , Value(InValue)
        , Style(InStyle)
        , Scale(InScale)
        , Radius(InRadius)
        , Location(InLocation)
    {
    }

    FSubjectHandle Owner = FSubjectHandle();
    float Value = 0;
    float Style = 0;
    float Scale = 1;
    float Radius = 0;
    FVector Location = FVector::ZeroVector;
};
