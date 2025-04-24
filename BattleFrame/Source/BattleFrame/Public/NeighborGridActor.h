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

    // NeighborGridActor成员函数实现
    void SphereTraceForSubjects
    (
        const FVector Origin,
        const float Radius,
        const bool bCheckVisibility,
        const FVector CheckOrigin,
        const float CheckRadius,
        const TArray<FSubjectHandle>& IgnoreSubjects,
        const FFilter Filter,
        TArray<FTraceResult>& Results
    )
    {
        if (LIKELY(NeighborGridComponent != nullptr))
        {
            TArray<FTraceResult> LocalResults;
            NeighborGridComponent->SphereTraceForSubjects(Origin, Radius, bCheckVisibility, CheckOrigin, CheckRadius, IgnoreSubjects, Filter, LocalResults);
            Results = MoveTemp(LocalResults);
        }
        else
        {
            Results.Empty();
        }
    }

    void SphereSweepForSubjects
    (
        const FVector Start,
        const FVector End,
        const float Radius,
        const bool bCheckVisibility,
        const float CheckRadius,
        const TArray<FSubjectHandle>& IgnoreSubjects,
        const FFilter Filter,
        TArray<FTraceResult>& Results
    )
    {
        if (LIKELY(NeighborGridComponent != nullptr))
        {
            TArray<FTraceResult> LocalResults;
            NeighborGridComponent->SphereSweepForSubjects(Start, End, Radius, bCheckVisibility, CheckRadius, IgnoreSubjects, Filter, LocalResults);
            Results = MoveTemp(LocalResults);
        }
        else
        {
            Results.Empty();
        }
    }

    void SectorTraceForSubject
    (
        const FVector Origin,
        const float Radius,
        const float Height,
        const FVector Direction,
        const float Angle,
        const bool bCheckVisibility,
        const float CheckRadius,
        const TArray<FSubjectHandle>& IgnoreSubjects,
        const FFilter Filter,
        FSubjectHandle& Result
    )
    {
        Result = FSubjectHandle();

        if (LIKELY(NeighborGridComponent != nullptr))
        {
            FSubjectHandle LocalResult;
            NeighborGridComponent->SectorTraceForSubject(Origin, Radius, Height, Direction, Angle, bCheckVisibility, CheckRadius, IgnoreSubjects, Filter, LocalResult);
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

