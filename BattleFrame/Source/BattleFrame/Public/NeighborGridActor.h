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
#include "Traits/TraceResult.h"
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
    void SphereTraceForSubjects(const FVector Location, const float Radius, TArray<FSubjectHandle> IgnoreSubjects, const FFilter Filter, TArray<FTraceResult>& Results)
    {
        if (LIKELY(Instance != nullptr && Instance->NeighborGridComponent != nullptr))
        {
            TArray<FTraceResult> LocalResults;
            Instance->NeighborGridComponent->SphereTraceForSubjects(Location, Radius, IgnoreSubjects, Filter, LocalResults);
            Results = MoveTemp(LocalResults); // 使用MoveTemp转移所有权
        }
        else
        {
            Results.Empty(); // 确保无效时清空结果
        }
    }

    void SphereSweepForSubjects(const FVector Start, const FVector End, float Radius, TArray<FSubjectHandle> IgnoreSubjects, const FFilter Filter, TArray<FTraceResult>& Results)
    {
        if (LIKELY(Instance != nullptr && Instance->NeighborGridComponent != nullptr))
        {
            TArray<FTraceResult> LocalResults;
            Instance->NeighborGridComponent->SphereSweepForSubjects(Start, End, Radius, IgnoreSubjects, Filter, LocalResults);
            Results = MoveTemp(LocalResults); // 使用MoveTemp转移所有权
        }
        else
        {
            Results.Empty();
        }
    }

    void CylinderExpandForSubject(const FVector Origin, float Radius, float Height, TArray<FSubjectHandle> IgnoreSubjects, const FFilter Filter, FSubjectHandle& Result)
    {
        Result = FSubjectHandle(); // 重置为无效句柄

        if (LIKELY(Instance != nullptr && Instance->NeighborGridComponent != nullptr))
        {
            FSubjectHandle LocalResult;
            Instance->NeighborGridComponent->CylinderExpandForSubject(Origin, Radius, Height, IgnoreSubjects, Filter, LocalResult);
            Result = MoveTemp(LocalResult); // 使用MoveTemp转移所有权
        }
    }

    void SectorExpandForSubject(const FVector Origin, float Radius, float Height, FVector Direction, float Angle, bool bCheckVisibility, TArray<FSubjectHandle> IgnoreSubjects, const FFilter Filter, FSubjectHandle& Result)
    {
        Result = FSubjectHandle();
        if (LIKELY(Instance != nullptr && Instance->NeighborGridComponent != nullptr))
        {
            FSubjectHandle LocalResult;
            Instance->NeighborGridComponent->SectorExpandForSubject(Origin, Radius, Height, Direction, Angle, bCheckVisibility, IgnoreSubjects, Filter, LocalResult);
            Result = MoveTemp(LocalResult);
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

