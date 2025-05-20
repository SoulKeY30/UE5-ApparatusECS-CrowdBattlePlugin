#include "BattleFrameActor.h"
#include "BattleFrameBattleControl.h"
#include "Traits/Prop.h"

ABattleFrameActor::ABattleFrameActor()
{
    Subjective = CreateDefaultSubobject<UBFSubjectiveActorComponent>("Subjective");
    Subjective->SetTrait(FProp{});

    PrimaryActorTick.bCanEverTick = true;
}

void ABattleFrameActor::BeginPlay()
{
    Super::BeginPlay();
}

void ABattleFrameActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABattleFrameActor::ReceiveDamage_Implementation(const FDmgResult& DmgResult)
{

}
