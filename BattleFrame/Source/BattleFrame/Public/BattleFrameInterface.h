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
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnAppear(const FAppearData& Data);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnTrace(const FTraceData& Data);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnMove(const FMoveData& Data);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnAttack(const FAttackData& Data);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnHit(const FHitData& Data);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnDeath(const FDeathData& Data);
};