//#pragma once
//
//#include "CoreMinimal.h"
//#include "Kismet/BlueprintAsyncActionBase.h"
//#include "BattleFrameAsyncNode.generated.h"
//
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTaskOutput);
//
//UCLASS()
//class BATTLEFRAME_API UBattleFrameAsyncAction : public UBlueprintAsyncActionBase
//{
//    GENERATED_BODY()
//
//public:
//    UPROPERTY(BlueprintAssignable)
//    FTaskOutput DoWork;
//
//    UPROPERTY(BlueprintAssignable)
//    FTaskOutput Completed;
//
//    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
//    static UBattleFrameAsyncAction* BattleFrameAsyncAction(const UObject* WorldContextObject);
//
//    virtual void Activate() override;
//};