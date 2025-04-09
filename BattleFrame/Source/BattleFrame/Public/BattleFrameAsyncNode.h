#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "BattleFrameAsyncNode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTaskOutput);


UCLASS()
class BATTLEFRAME_API UBattleFrameAsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintAssignable)
	FTaskOutput DoWork;

	UPROPERTY(BlueprintAssignable)
	FTaskOutput Completed;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UBattleFrameAsyncAction* BattleFrameAsyncAction();

	virtual void Activate() override;
};

class BattleFrameAsyncTask : public FNonAbandonableTask
{
public:
	
	// constructor
	BattleFrameAsyncTask(UBattleFrameAsyncAction* AsyncActionInstance);

	// destructor
	~BattleFrameAsyncTask();

	// fixed macro
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(BattleFrameAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	UBattleFrameAsyncAction* Owner = nullptr;

	void DoWork();
};