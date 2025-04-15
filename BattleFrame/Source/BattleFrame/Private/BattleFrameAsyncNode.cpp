//#include "BattleFrameAsyncNode.h"
//#include "Async/Async.h"
//
//UBattleFrameAsyncAction* UBattleFrameAsyncAction::BattleFrameAsyncAction(const UObject* WorldContextObject)
//{
//    UBattleFrameAsyncAction* AsyncAction = NewObject<UBattleFrameAsyncAction>();
//    AsyncAction->RegisterWithGameInstance(WorldContextObject ? WorldContextObject->GetWorld() : nullptr);
//
//    return AsyncAction;
//}
//
//void UBattleFrameAsyncAction::Activate()
//{
//    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
//    {
//        DoWork.Broadcast();
//
//        AsyncTask(ENamedThreads::GameThread, [this]()
//        {
//            Completed.Broadcast();
//            SetReadyToDestroy();
//        });
//    });
//}