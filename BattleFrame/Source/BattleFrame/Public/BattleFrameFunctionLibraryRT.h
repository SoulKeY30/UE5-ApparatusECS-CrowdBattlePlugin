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
#include "Traits/AvoGroup.h"
#include "Traits/Team.h"
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
    static void CalculateThreadsCountAndBatchSize(int32 IterableNum, int32 MaxThreadsAllowed, int32 MinBatchSizeAllowed, int32& ThreadsCount, int32& BatchSize);

    static void SetRecordSubTypeTraitByIndex(int32 Index, FSubjectRecord& SubjectRecord);
    static void SetRecordSubTypeTraitByEnum(EESubType SubType, FSubjectRecord& SubjectRecord);
    static void SetSubjectSubTypeTraitByIndex(int32 Index, FSubjectHandle SubjectHandle);
    static void IncludeSubTypeTraitByIndex(int32 Index, FFilter& Filter);

    FORCEINLINE static void SetSubjectAvoGroupTraitByIndex(int32 Index, FSubjectHandle SubjectHandle)
    {
        switch (Index)
        {
            default:
                SubjectHandle.SetTrait(FAvoGroup0());
                break;
            case 0:
                SubjectHandle.SetTrait(FAvoGroup0());
                break;
            case 1:
                SubjectHandle.SetTrait(FAvoGroup1());
                break;
            case 2:
                SubjectHandle.SetTrait(FAvoGroup2());
                break;
            case 3:
                SubjectHandle.SetTrait(FAvoGroup3());
                break;
            case 4:
                SubjectHandle.SetTrait(FAvoGroup4());
                break;
            case 5:
                SubjectHandle.SetTrait(FAvoGroup5());
                break;
            case 6:
                SubjectHandle.SetTrait(FAvoGroup6());
                break;
            case 7:
                SubjectHandle.SetTrait(FAvoGroup7());
                break;
            case 8:
                SubjectHandle.SetTrait(FAvoGroup8());
                break;
            case 9:
                SubjectHandle.SetTrait(FAvoGroup9());
                break;
        }
    };

    FORCEINLINE static void SetSubjectTeamTraitByIndex(int32 Index, FSubjectHandle SubjectHandle) 
    {
        switch (Index)
        {
            default:
                SubjectHandle.SetTrait(FTeam0());
                break;
            case 0:
                SubjectHandle.SetTrait(FTeam0());
                break;
            case 1:
                SubjectHandle.SetTrait(FTeam1());
                break;
            case 2:
                SubjectHandle.SetTrait(FTeam2());
                break;
            case 3:
                SubjectHandle.SetTrait(FTeam3());
                break;
            case 4:
                SubjectHandle.SetTrait(FTeam4());
                break;
            case 5:
                SubjectHandle.SetTrait(FTeam5());
                break;
            case 6:
                SubjectHandle.SetTrait(FTeam6());
                break;
            case 7:
                SubjectHandle.SetTrait(FTeam7());
                break;
            case 8:
                SubjectHandle.SetTrait(FTeam8());
                break;
            case 9:
                SubjectHandle.SetTrait(FTeam9());
                break;
        }
    };

    FORCEINLINE static void SetRecordAvoGroupTraitByIndex(int32 Index, FSubjectRecord& SubjectRecord)
    {
        switch (Index)
        {
            default:
                SubjectRecord.SetTrait(FAvoGroup0());
                break;
            case 0:
                SubjectRecord.SetTrait(FAvoGroup0());
                break;
            case 1:
                SubjectRecord.SetTrait(FAvoGroup1());
                break;
            case 2:
                SubjectRecord.SetTrait(FAvoGroup2());
                break;
            case 3:
                SubjectRecord.SetTrait(FAvoGroup3());
                break;
            case 4:
                SubjectRecord.SetTrait(FAvoGroup4());
                break;
            case 5:
                SubjectRecord.SetTrait(FAvoGroup5());
                break;
            case 6:
                SubjectRecord.SetTrait(FAvoGroup6());
                break;
            case 7:
                SubjectRecord.SetTrait(FAvoGroup7());
                break;
            case 8:
                SubjectRecord.SetTrait(FAvoGroup8());
                break;
            case 9:
                SubjectRecord.SetTrait(FAvoGroup9());
                break;
        }
    };

    FORCEINLINE static void SetRecordTeamTraitByIndex(int32 Index, FSubjectRecord& SubjectRecord)
    {
        switch (Index)
        {
            default:
                SubjectRecord.SetTrait(FTeam0());
                break;
            case 0:
                SubjectRecord.SetTrait(FTeam0());
                break;
            case 1:
                SubjectRecord.SetTrait(FTeam1());
                break;
            case 2:
                SubjectRecord.SetTrait(FTeam2());
                break;
            case 3:
                SubjectRecord.SetTrait(FTeam3());
                break;
            case 4:
                SubjectRecord.SetTrait(FTeam4());
                break;
            case 5:
                SubjectRecord.SetTrait(FTeam5());
                break;
            case 6:
                SubjectRecord.SetTrait(FTeam6());
                break;
            case 7:
                SubjectRecord.SetTrait(FTeam7());
                break;
            case 8:
                SubjectRecord.SetTrait(FTeam8());
                break;
            case 9:
                SubjectRecord.SetTrait(FTeam9());
                break;
        }
    };

    FORCEINLINE static void IncludeAvoGroupTraitByIndex(int32 Index, FFilter& Filter)
    {
        switch (Index)
        {
            case 0:
                Filter.Include<FAvoGroup0>();
                break;

            case 1:
                Filter.Include<FAvoGroup1>();
                break;

            case 2:
                Filter.Include<FAvoGroup2>();
                break;

            case 3:
                Filter.Include<FAvoGroup3>();
                break;

            case 4:
                Filter.Include<FAvoGroup4>();
                break;

            case 5:
                Filter.Include<FAvoGroup5>();
                break;

            case 6:
                Filter.Include<FAvoGroup6>();
                break;

            case 7:
                Filter.Include<FAvoGroup7>();
                break;

            case 8:
                Filter.Include<FAvoGroup8>();
                break;

            case 9:
                Filter.Include<FAvoGroup9>();
                break;
        }
    };

    FORCEINLINE static void ExcludeAvoGroupTraitByIndex(int32 Index, FFilter& Filter)
    {
        switch (Index)
        {
            case 0:
                Filter.Exclude<FAvoGroup0>();
                break;

            case 1:
                Filter.Exclude<FAvoGroup1>();
                break;

            case 2:
                Filter.Exclude<FAvoGroup2>();
                break;

            case 3:
                Filter.Exclude<FAvoGroup3>();
                break;

            case 4:
                Filter.Exclude<FAvoGroup4>();
                break;

            case 5:
                Filter.Exclude<FAvoGroup5>();
                break;

            case 6:
                Filter.Exclude<FAvoGroup6>();
                break;

            case 7:
                Filter.Exclude<FAvoGroup7>();
                break;

            case 8:
                Filter.Exclude<FAvoGroup8>();
                break;

            case 9:
                Filter.Exclude<FAvoGroup9>();
                break;
        }
    };
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
