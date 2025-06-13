// BattleFrameInterface.h
#pragma once

#include "CoreMinimal.h"
#include "BattleFrameStructs.h"
#include "UObject/Interface.h"
#include "BattleFrameInterface.generated.h"

UINTERFACE(MinimalAPI)
class UBattleFrameInterface : public UInterface
{
    GENERATED_BODY()
};

class BATTLEFRAME_API IBattleFrameInterface
{
    GENERATED_BODY()

public:

    // 接受伤害时触发
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void OnAppear(const FAppearData& Data);

    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void OnSleep(const FSleepData& Data);

    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void OnPatrol(const FPatrolData& Data);

    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void OnFalling(const FFallingData& Data);

    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void OnTraceSucceed(const FTraceSucceedData& Data);

    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void OnChase(const FChaseData& Data);

    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void OnAttackBegin(const FAttackBeginData& Data);

    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void OnAttackHit(const FAttackHitData& Data);

    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void OnAttackCoolling(const FAttackCoollingData& Data);

    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void OnAttackEnd(const FAttackEnd& Data);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnHit(const FHitData& Data);

    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void OnDeathBegin(const FDeathBeginData& Data);

    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    //void OnDeathEnd(const FDeathEnd& Data);

};