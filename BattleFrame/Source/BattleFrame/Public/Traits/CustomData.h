#pragma once

#include "CoreMinimal.h"
#include "Math/Vector.h"     // FVector依赖
#include "Math/Rotator.h"    // FRotator依赖
#include "UObject/Class.h"   // UObject基础支持
#include "CustomData.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FCustomData
{
    GENERATED_BODY()

public:
    // 基础类型映射（保持原样）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataMaps")
    TMap<FString, bool> BoolMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataMaps")
    TMap<FString, int32> Int32Map;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataMaps")
    TMap<FString, float> FloatMap;

    // UE数学类型映射
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataMaps")
    TMap<FString, FVector> VectorMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataMaps")
    TMap<FString, FRotator> RotatorMap;

    // UObject通用对象映射（核心修改部分）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataMaps", meta = (AllowedClasses = "/Script/CoreUObject.Object"))
    TMap<FString, TObjectPtr<UObject>> ObjectMap;
};
