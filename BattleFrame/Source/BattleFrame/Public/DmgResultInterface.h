// DmgResultInterface.h
#pragma once

#include "CoreMinimal.h"
#include "BattleFrameStructs.h"
#include "UObject/Interface.h"
#include "DmgResultInterface.generated.h"

UINTERFACE(MinimalAPI)
class UDmgResultInterface : public UInterface
{
    GENERATED_BODY()
};

class BATTLEFRAME_API IDmgResultInterface
{
    GENERATED_BODY()

public:

    // 接受伤害时触发
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void ReceiveDamage(const FDmgResult& DmgResult);

};