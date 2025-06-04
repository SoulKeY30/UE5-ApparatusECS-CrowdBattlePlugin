#pragma once

#include "CoreMinimal.h"
#include "SubjectiveActorComponent.h"
#include "BattleFrameStructs.h"
#include "AgentConfigDataAsset.h"
#include "BFSubjectiveActorComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent))
class BATTLEFRAME_API UBFSubjectiveActorComponent : public USubjectiveActorComponent
{
    GENERATED_BODY()

public:
    UBFSubjectiveActorComponent();

    // Initialize all required traits
    void InitializeTraits(AActor* OwnerActor);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void AsyncTransformActorToSubject(AActor* OwnerActor);
};
