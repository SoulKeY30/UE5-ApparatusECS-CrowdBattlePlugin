/*
 * APPARATIST
 * Created: 2023-02-02 16:26:26
 * Author: Vladislav Dmitrievich Turbanov (vladislav@turbanov.ru)
 * Community forums: https://talk.turbanov.ru
 * Copyright 2019 - 2023, SP Vladislav Dmitrievich Turbanov
 * Made in Russia, Moscow City, Chekhov City
 */

 /*
  * BattleFrame
  * Refactor: 2025
  * Author: Leroy Works
  */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NeighborGridComponent.h"
#include "BattleFrameStructs.h"
#include "NeighborGridActor.generated.h"

#define BUBBLE_DEBUG 0

/**
 * A simple and performant collision detection and decoupling for spheres.
 */
UCLASS(Category = "NeighborGrid")
class BATTLEFRAME_API ANeighborGridActor : public AActor
{
    GENERATED_BODY()

private:


    /**
     * The main bubble cage component.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bubble Cage", Meta = (AllowPrivateAccess = "true"))
    UNeighborGridComponent* NeighborGridComponent = nullptr;

protected:

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
    }

    virtual void BeginDestroy() override
    {
        Super::BeginDestroy();
    }


public:

    ANeighborGridActor();

    virtual void OnConstruction(const FTransform& Transform) override;


    UNeighborGridComponent* GetComponent()
    {
        return NeighborGridComponent;
    }
};

