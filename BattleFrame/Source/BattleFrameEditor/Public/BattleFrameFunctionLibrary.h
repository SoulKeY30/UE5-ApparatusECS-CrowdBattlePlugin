#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "BattleFrameFunctionLibrary.generated.h"

class UNiagaraSystem;
class UStaticMesh;

UCLASS()
class BATTLEFRAMEEDITOR_API UBattleFrameFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "BattleFrame Functions", meta = (DisplayName = "Duplicate Class Asset", WorldContext = "WorldContextObject"))
    static UClass* DuplicateClassAsset(
        UObject* WorldContextObject,
        TSubclassOf<UObject> SourceClass,
        FString NewClassName,
        FString PackagePath = "/Game/Blueprints/Duplicated"
    );

    UFUNCTION(BlueprintCallable, Category = "BattleFrame Functions", meta = (DisplayName = "Set Class Default Properties", WorldContext = "WorldContextObject"))
    static void SetClassDefaultProperties(
        UObject* WorldContextObject,
        TSubclassOf<ANiagaraSubjectRenderer> TargetClass,
        UStaticMesh* NewMesh,
        UNiagaraSystem* NewNiagaraSystem,
        int32 SubType
    );

};
