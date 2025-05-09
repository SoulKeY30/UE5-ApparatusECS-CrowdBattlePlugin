// DmgResultInterface.h
#pragma once

#include "CoreMinimal.h"
#include "BattleFrameStructs.h"
#include "DmgResultInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UDmgResultInterface : public UInterface
{
    GENERATED_BODY()
};

class BATTLEFRAME_API IDmgResultInterface
{
    GENERATED_BODY()

public:

    // 接受伤害时触发
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "MyInterface")
    void ReceiveDamage(const FDmgResult& DmgResult);

};