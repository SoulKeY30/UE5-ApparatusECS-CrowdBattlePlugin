#pragma once

#include "CoreMinimal.h"
#include "BFSubjectiveActorComponent.h"
#include "DmgResultInterface.h"
#include "BattleFrameStructs.h"
#include "BattleFrameActor.generated.h"

UCLASS()
class BATTLEFRAME_API ABattleFrameActor : public AActor, public IDmgResultInterface
{
    GENERATED_BODY()

public:
    ABattleFrameActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component)
    UBFSubjectiveActorComponent* Subjective = nullptr;

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void ReceiveDamage_Implementation(const FDmgResult& DmgResult) override;
};
