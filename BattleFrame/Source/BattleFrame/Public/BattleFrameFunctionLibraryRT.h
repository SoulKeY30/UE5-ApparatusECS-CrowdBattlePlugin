// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SubjectHandle.h"
#include "Traits/SubType.h"
#include "Traits/Patrol.h"
#include "Traits/Collider.h"
#include "Traits/Trace.h"
#include "Traits/Located.h"
#include "Traits/DmgSphere.h"
#include "Traits/Debuff.h"
#include "BattleFrameEnums.h"
#include "BattleFrameStructs.h"
#include "BattleFrameFunctionLibraryRT.generated.h"

class ABattleFrameBattleControl;
class ANeighborGridActor;

UCLASS()
class BATTLEFRAME_API UBattleFrameFunctionLibraryRT : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (AutoCreateRefTerm = "Origin, CheckOrigin, SortOrigin, IgnoreSubjects, Filter"))
    static void SphereTraceForSubjects
    (
        bool& Hit,
        TArray<FTraceResult>& TraceResults,
        ANeighborGridActor* NeighborGridActor = nullptr,
        int32 KeepCount = -1,
        UPARAM(ref) const FVector& Origin = FVector(0, 0, 0),
        float Radius = 0.f,
        bool bCheckVisibility = false,
        UPARAM(ref) const FVector& CheckOrigin = FVector(0, 0, 0),
        float CheckRadius = 0.f,
        ESortMode SortMode = ESortMode::None,
        UPARAM(ref) const FVector& SortOrigin = FVector(0, 0, 0),
        UPARAM(ref) const FSubjectArray& IgnoreSubjects = FSubjectArray(),
        UPARAM(ref) const FFilter& Filter = FFilter()
    );

    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (AutoCreateRefTerm = "Start, End, CheckOrigin, SortOrigin, IgnoreSubjects, Filter"))
    static void SphereSweepForSubjects
    (
        bool& Hit,
        TArray<FTraceResult>& TraceResults,
        ANeighborGridActor* NeighborGridActor = nullptr,
        int32 KeepCount = -1,
        UPARAM(ref) const FVector& Start = FVector(0, 0, 0),
        UPARAM(ref) const FVector& End = FVector(0, 0, 0),
        float Radius = 0.f,
        bool bCheckVisibility = false, 
        UPARAM(ref) const FVector& CheckOrigin = FVector(0, 0, 0),
        float CheckRadius = 0.f,
        ESortMode SortMode = ESortMode::None,
        UPARAM(ref) const FVector& SortOrigin = FVector(0, 0, 0),
        UPARAM(ref) const FSubjectArray& IgnoreSubjects = FSubjectArray(),
        UPARAM(ref) const FFilter& Filter = FFilter()
    );

    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (AutoCreateRefTerm = "Origin, Direction, CheckOrigin, SortOrigin, IgnoreSubjects, Filter"))
    static void SectorTraceForSubjects
    (
        bool& Hit,
        TArray<FTraceResult>& TraceResults,
        ANeighborGridActor* NeighborGridActor = nullptr,
        int32 KeepCount = -1,
        UPARAM(ref) const FVector& Origin = FVector(0, 0, 0),
        float Radius = 300.f,
        float Height = 100.f,
        UPARAM(ref) const FVector& Direction = FVector(1, 0, 0),
        float Angle = 360.f,
        bool bCheckVisibility = false, 
        UPARAM(ref) const FVector& CheckOrigin = FVector(0, 0, 0),
        float CheckRadius = 0.f,
        ESortMode SortMode = ESortMode::None,
        UPARAM(ref) const FVector& SortOrigin = FVector(0, 0, 0),
        UPARAM(ref) const FSubjectArray& IgnoreSubjects = FSubjectArray(),
        UPARAM(ref) const FFilter& Filter = FFilter()
    );

    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (AutoCreateRefTerm = "Start, End"))
    static void SphereSweepForObstacle
    (
        bool& Hit,
        FTraceResult& TraceResult,
        ANeighborGridActor* NeighborGridActor = nullptr,
        UPARAM(ref) const FVector& Start = FVector(0, 0, 0),
        UPARAM(ref) const FVector& End = FVector(0, 0, 0),
        float Radius = 0.f
    );

    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (AutoCreateRefTerm = "Subjects, IgnoreSubjects, DmgInstigator, HitFromLocation, DmgSphere, Debuff"))
    static void ApplyDamageToSubjects
    (
        TArray<FDmgResult>& DamageResults,
        ABattleFrameBattleControl* BattleControl = nullptr,
        UPARAM(ref) const FSubjectArray& Subjects = FSubjectArray(),
        UPARAM(ref) const FSubjectArray& IgnoreSubjects = FSubjectArray(),
        UPARAM(ref) const FSubjectHandle& DmgInstigator = FSubjectHandle(),
        UPARAM(ref) const FVector& HitFromLocation = FVector(0, 0, 0),
        UPARAM(ref) const FDmgSphere& DmgSphere = FDmgSphere(),
        UPARAM(ref) const FDebuff& Debuff = FDebuff()
    );

    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (DisplayName = "Sort Subjects By Distance", Keywords = "Sort Distance Subject", AutoCreateRefTerm = "TraceResults"))
    static void SortSubjectsByDistance
    (
        UPARAM(ref) TArray<FTraceResult>& TraceResults,
        const FVector& SortOrigin,
        const ESortMode SortMode = ESortMode::NearToFar
    );

    UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Convert to SubjectHandles", CompactNodeTitle = "->", BlueprintAutocast), Category = "BattleFrame")
    static TArray<FSubjectHandle> ConvertDmgResultsToSubjectHandles(const TArray<FDmgResult>& DmgResults);

    UFUNCTION(BlueprintCallable, BlueprintPure,meta = (DisplayName = "Convert to SubjectHandles",CompactNodeTitle = "->",BlueprintAutocast),Category = "BattleFrame")
    static TArray<FSubjectHandle> ConvertTraceResultsToSubjectHandles(const TArray<FTraceResult>& TraceResults);

    UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Convert to SubjectArray", CompactNodeTitle = "->", BlueprintAutocast), Category = "BattleFrame")
    static FSubjectArray ConvertDmgResultsToSubjectArray(const TArray<FDmgResult>& DmgResults);

    UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Convert to SubjectArray", CompactNodeTitle = "->", BlueprintAutocast), Category = "BattleFrame")
    static FSubjectArray ConvertTraceResultsToSubjectArray(const TArray<FTraceResult>& TraceResults);

    UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Convert to SubjectArray", CompactNodeTitle = "->", BlueprintAutocast), Category = "BattleFrame")
    static FSubjectArray ConvertSubjectHandlesToSubjectArray(const TArray<FSubjectHandle>& SubjectHandles);

    static FVector FindNewPatrolGoalLocation(const FPatrol& Patrol, const FCollider& Collider, const FTrace& Trace, const FLocated& Located, int32 MaxAttempts);
    static void SetRecordSubTypeTraitByIndex(int32 Index, FSubjectRecord& SubjectRecord);
    static void SetRecordSubTypeTraitByEnum(EESubType SubType, FSubjectRecord& SubjectRecord);
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
