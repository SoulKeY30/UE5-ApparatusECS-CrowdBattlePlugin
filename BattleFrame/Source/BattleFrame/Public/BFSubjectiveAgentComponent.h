#pragma once

#include "CoreMinimal.h"
#include "SubjectiveActorComponent.h"
#include "BattleFrameStructs.h"
#include "AgentConfigDataAsset.h"
#include "BattleFrameBattleControl.h"
#include "BFSubjectiveAgentComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent))
class BATTLEFRAME_API UBFSubjectiveAgentComponent : public USubjectiveActorComponent
{
    GENERATED_BODY()

public:
    UBFSubjectiveAgentComponent();

    UFUNCTION(BlueprintCallable)
    void InitializeTraits(AActor* OwnerActor);

    UFUNCTION(BlueprintCallable)
    void SyncTransformSubjectToActor(AActor* OwnerActor);

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TSoftObjectPtr<UAgentConfigDataAsset> AgentConfigAsset;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 TeamIndex = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector2D LaunchVelocity = FVector2d(0, 0);

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FSpawnerMult Multipliers = FSpawnerMult();

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bAutoInitWithDataAsset = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bSyncTransformSubjectToActor = true;

    UWorld* CurrentWorld = nullptr;
    AMechanism* Mechanism = nullptr;
    ABattleFrameBattleControl* BattleControl = nullptr;

    EFlagmarkBit RegisterMultipleFlag = EFlagmarkBit::M;

protected:

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
