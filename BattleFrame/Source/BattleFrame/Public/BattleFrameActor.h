#pragma once

#include "CoreMinimal.h"
#include "BFSubjectiveActorComponent.h"
#include "BattleFrameInterface.h"
#include "BattleFrameStructs.h"
#include "BattleFrameActor.generated.h"

UCLASS()
class BATTLEFRAME_API ABattleFrameActor : public AActor, public IBattleFrameInterface
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
    virtual void OnHit_Implementation(const FHitData& Data) override;
};
