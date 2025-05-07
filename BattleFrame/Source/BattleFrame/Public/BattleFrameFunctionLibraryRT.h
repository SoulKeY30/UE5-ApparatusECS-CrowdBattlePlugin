// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SubjectHandle.h"
#include "Traits/SubType.h"
#include "Traits/TraceResult.h"
#include "Traits/Patrol.h"
#include "Traits/Collider.h"
#include "Traits/Trace.h"
#include "Traits/Located.h"
#include "Traits/BattleFrameEnums.h"
#include "BattleFrameFunctionLibraryRT.generated.h"

class ABattleFrameBattleControl;
class ANeighborGridActor;


UCLASS()
class BATTLEFRAME_API UBattleFrameFunctionLibraryRT : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (AutoCreateRefTerm = "IgnoreSubjects"))
    static void SphereTraceForSubjects
    (
        ANeighborGridActor* NeighborGridActor,
        int32 KeepCount,
        FVector Origin,
        float Radius,
        bool bCheckVisibility,
        FVector CheckOrigin,
        float CheckRadius,
        ESortMode SortMode,
        const FVector SortOrigin,
        UPARAM(ref) const TArray<FSubjectHandle>& IgnoreSubjects,
        UPARAM(ref) const FFilter& Filter,
        bool& Hit,
        TArray<FTraceResult>& TraceResults
    );

    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (AutoCreateRefTerm = "IgnoreSubjects"))
    static void SphereSweepForSubjects
    (
        ANeighborGridActor* NeighborGridActor,
        int32 KeepCount,
        FVector Start,
        FVector End,
        float Radius,
        bool bCheckVisibility, 
        const FVector CheckOrigin,
        float CheckRadius,
        ESortMode SortMode,
        const FVector SortOrigin,
        UPARAM(ref) const TArray<FSubjectHandle>& IgnoreSubjects,
        UPARAM(ref) const FFilter& Filter,
        bool& Hit,
        TArray<FTraceResult>& TraceResults
    );

    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (AutoCreateRefTerm = "IgnoreSubjects"))
    static void SectorTraceForSubjects
    (
        ANeighborGridActor* NeighborGridActor,
        int32 KeepCount,
        FVector Origin,
        float Radius,
        float Height,
        FVector Direction,
        float Angle,
        bool bCheckVisibility, 
        const FVector CheckOrigin,
        float CheckRadius,
        ESortMode SortMode,
        const FVector SortOrigin,
        UPARAM(ref) const TArray<FSubjectHandle>& IgnoreSubjects,
        UPARAM(ref) const FFilter& Filter,
        bool& Hit,
        TArray<FTraceResult>& TraceResults
    );

    UFUNCTION(BlueprintCallable, Category = "BattleFrame")
    static void SphereSweepForObstacle
    (
        ANeighborGridActor* NeighborGridActor, 
        FVector Start, 
        FVector End, 
        float Radius, 
        bool& Hit, 
        FTraceResult& TraceResult
    );

    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (AutoCreateRefTerm = "IgnoreSubjects"))
    static void ApplyDamageToSubjects
    (
        ABattleFrameBattleControl* BattleControl,
        UPARAM(ref) const TArray<FSubjectHandle>& Subjects,
        UPARAM(ref) const TArray<FSubjectHandle>& IgnoreSubjects,
        FSubjectHandle DmgInstigator,
        FVector HitFromLocation,
        FDmgSphere DmgSphere,
        FDebuff Debuff,
        TArray<FDmgResult>& DamageResults
    );

    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (DisplayName = "Sort Subjects By Distance", Keywords = "Sort Distance Subject"))
    static void SortSubjectsByDistance
    (
        UPARAM(ref) TArray<FTraceResult>& TraceResults,
        const FVector& SortOrigin,
        ESortMode SortMode = ESortMode::NearToFar
    );

    UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Convert to SubjectHandles", CompactNodeTitle = "->", BlueprintAutocast), Category = "BattleFrame")
    static TArray<FSubjectHandle> ConvertDmgResultsToSubjectHandles(const TArray<FDmgResult>& DmgResults);

    UFUNCTION(BlueprintCallable, BlueprintPure,meta = (DisplayName = "Convert to SubjectHandles",CompactNodeTitle = "->",BlueprintAutocast),Category = "BattleFrame")
    static TArray<FSubjectHandle> ConvertTraceResultsToSubjectHandles(const TArray<FTraceResult>& TraceResults);

    static FVector FindNewPatrolGoalLocation(const FPatrol& Patrol, const FCollider& Collider, const FTrace& Trace, const FLocated& Located, int32 MaxAttempts);
    static void SetRecordSubTypeTraitByIndex(int32 Index, FSubjectRecord& SubjectRecord);
    static void SetRecordSubTypeTraitByEnum(ESubType SubType, FSubjectRecord& SubjectRecord);
    static void SetSubjectSubTypeTraitByIndex(int32 Index, FSubjectHandle SubjectHandle);
    static void IncludeSubTypeTraitByIndex(int32 Index, FFilter& Filter);
    static void CalculateThreadsCountAndBatchSize(int32 IterableNum, int32 MaxThreadsAllowed, int32 MinBatchSizeAllowed, int32& ThreadsCount, int32& BatchSize);
    static void SetSubjectAvoGroupTraitByIndex(int32 Index, FSubjectHandle SubjectHandle);
    static void SetSubjectTeamTraitByIndex(int32 Index, FSubjectHandle SubjectHandle);
    static void SetRecordAvoGroupTraitByIndex(int32 Index, FSubjectRecord& SubjectRecord);
    static void SetRecordTeamTraitByIndex(int32 Index, FSubjectRecord& SubjectRecord);
    static void IncludeAvoGroupTraitByIndex(int32 Index, FFilter& Filter);
    static void ExcludeAvoGroupTraitByIndex(int32 Index, FFilter& Filter);
};

//-------------------------------Async Trace-------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAsyncTraceOutput, bool, Hit, const TArray<FTraceResult>&, TraceResults);

UCLASS()
class BATTLEFRAME_API USphereSweepForSubjectsAsyncAction : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintAssignable)
    FAsyncTraceOutput Completed;

    TWeakObjectPtr<ANeighborGridActor> NeighborGridActor;
    int32 KeepCount;
    FVector Start;
    FVector End;
    float Radius;
    bool bCheckVisibility;
    FVector CheckOrigin;
    float CheckRadius;
    ESortMode SortMode;
    FVector SortOrigin;
    FFilter Filter;
    TArray<FSubjectHandle> IgnoreSubjects;
    bool Hit;
    TArray<FTraceResult> TempResults;
    TArray<FTraceResult> Results;

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm = "IgnoreSubjects"))
    static USphereSweepForSubjectsAsyncAction* SphereSweepForSubjectsAsync
    (
        const UObject* WorldContextObject, 
        ANeighborGridActor* NeighborGridActor, 
        const int32 KeepCount,
        const FVector Start, 
        const FVector End, 
        const float Radius,
        const bool bCheckVisibility,
        const FVector CheckOrigin,
        const float CheckRadius,
        const ESortMode SortMode,
        const FVector SortOrigin, 
        const TArray<FSubjectHandle>& IgnoreSubjects, 
        const FFilter Filter
    );

    virtual void Activate() override;
};
