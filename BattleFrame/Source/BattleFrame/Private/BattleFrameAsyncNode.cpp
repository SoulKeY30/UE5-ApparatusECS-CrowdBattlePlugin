// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleFrameAsyncNode.h"

UBattleFrameAsyncAction* UBattleFrameAsyncAction::BattleFrameAsyncAction()
{
	UBattleFrameAsyncAction* AsyncAction = NewObject<UBattleFrameAsyncAction>();
	return AsyncAction;
}

void UBattleFrameAsyncAction::Activate()
{
	(new FAutoDeleteAsyncTask<BattleFrameAsyncTask>(this))->StartBackgroundTask();
}

BattleFrameAsyncTask::BattleFrameAsyncTask(UBattleFrameAsyncAction* AsyncActionInstance)
{
	Owner = AsyncActionInstance;
}

BattleFrameAsyncTask::~BattleFrameAsyncTask()
{
	Owner->Completed.Broadcast();
	Owner->SetReadyToDestroy();
}

void BattleFrameAsyncTask::DoWork()
{
	Owner->DoWork.Broadcast();
}
