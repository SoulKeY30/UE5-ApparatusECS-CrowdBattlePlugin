﻿/*
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
     * The singleton instance of the cage.
     */
    static ANeighborGridActor* Instance;

    /**
     * The main bubble cage component.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bubble Cage", Meta = (AllowPrivateAccess = "true"))
    UNeighborGridComponent* NeighborGridComponent = nullptr;

protected:

    virtual void BeginPlay() override
    {
        Super::BeginPlay();

        Instance = this;
    }

    virtual void BeginDestroy() override
    {
        if (LIKELY(Instance == this))
        {
            Instance = nullptr;
        }

        Super::BeginDestroy();
    }


public:

    ANeighborGridActor();

    virtual void OnConstruction(const FTransform& Transform) override;

    /**
     * Get the global instance of the cage.
     */
    static FORCEINLINE ANeighborGridActor* GetInstance()
    {
        return Instance;
    }

    /**
     * Get the actual bubble cage component.
     */
    UNeighborGridComponent* GetComponent()
    {
        return NeighborGridComponent;
    }

    /**
     * Get overlapping spheres for the specified location.
     */
    UFUNCTION(BlueprintCallable)
    void SphereTraceForSubjects(const FVector Location, const float Radius, const FFilter Filter, TArray<FSubjectHandle>& Results)
    {
        Results.Reset();

        if (LIKELY(Instance != nullptr && Instance->NeighborGridComponent != nullptr))
        {
            Instance->NeighborGridComponent->SphereTraceForSubjects(Location, Radius, Filter, Results);
        }
    }

    UFUNCTION(BlueprintCallable)
    void SphereSweepForSubjects(const FVector Start, const FVector End, float Radius, const FFilter Filter, TArray<FSubjectHandle>& Results)
    {
        Results.Reset();

        if (LIKELY(Instance != nullptr && Instance->NeighborGridComponent != nullptr))
        {
            Instance->NeighborGridComponent->SphereSweepForSubjects(Start, End, Radius, Filter, Results);
        }
    }

    UFUNCTION(BlueprintCallable)
    void SphereExpandForSubject(const FVector Origin, float Radius, float Height, const FFilter Filter, FSubjectHandle& Result)
    {
        Result = FSubjectHandle();

        if (LIKELY(Instance != nullptr && Instance->NeighborGridComponent != nullptr))
        {
            Instance->NeighborGridComponent->SphereExpandForSubject(Origin, Radius, Height, Filter, Result);
        }
    }

    /**
     * Re-fill the cage with bubbles.
     */
    UFUNCTION(BlueprintCallable)
    static void Update()
    {
        if (UNLIKELY(Instance == nullptr)) return;
        Instance->NeighborGridComponent->Update();
    }

    /**
     * Decouple the bubbles within the cage.
     */
    UFUNCTION(BlueprintCallable)
    static void Decouple()
    {
        if (UNLIKELY(Instance == nullptr)) return;
        Instance->NeighborGridComponent->Decouple();
    }

    /**
     * Re-register and decouple the bubbles.
     */
    UFUNCTION(BlueprintCallable)
    static void Evaluate()
    {
        if (UNLIKELY(Instance == nullptr)) return;
        Instance->NeighborGridComponent->Evaluate();
    }
};