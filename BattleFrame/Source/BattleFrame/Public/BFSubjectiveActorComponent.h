#pragma once

#include "CoreMinimal.h"
#include "SubjectiveActorComponent.h"
#include "BattleFrameStructs.h"
#include "BFSubjectiveActorComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent))
class BATTLEFRAME_API UBFSubjectiveActorComponent : public USubjectiveActorComponent
{
    GENERATED_BODY()

public:
    UBFSubjectiveActorComponent();

    // Initialize all required traits
    UFUNCTION(BlueprintCallable, Category = "BattleFrame|Subjective")
    void InitializeTraits(AActor* OwnerActor);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void UpdateTraits();
};
