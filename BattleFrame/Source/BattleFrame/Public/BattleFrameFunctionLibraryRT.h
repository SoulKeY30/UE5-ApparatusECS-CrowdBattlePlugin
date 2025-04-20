// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SubjectHandle.h"
#include "Traits/SubType.h"
#include "Traits/TraceResult.h"
#include "BattleFrameFunctionLibraryRT.generated.h"

class ABattleFrameBattleControl;
class ANeighborGridActor;

UENUM(BlueprintType)
enum class ESortMode : uint8
{
    FarToNear UMETA(DisplayName = "Far to Near", ToolTip = "从远到近"),
    NearToFar UMETA(DisplayName = "Near to Far", ToolTip = "从近到远")
};

UCLASS()
class BATTLEFRAME_API UBattleFrameFunctionLibraryRT : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (AutoCreateRefTerm = "IgnoreSubjects"))
    static void SphereTraceForSubjects(
        ANeighborGridActor* NeighborGridActor,
        FVector Location,
        float Radius,
        const TArray<FSubjectHandle>& IgnoreSubjects,
        FFilter Filter,
        TArray<FTraceResult>& Results
    );


    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (AutoCreateRefTerm = "IgnoreSubjects"))
    static void SphereSweepForSubjects(
        ANeighborGridActor* NeighborGridActor,
        FVector Start,
        FVector End,
        float Radius,
        const TArray<FSubjectHandle>& IgnoreSubjects,
        FFilter Filter,
        TArray<FTraceResult>& Results
    );

    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (AutoCreateRefTerm = "IgnoreSubjects"))
    static void CylinderExpandForSubject(
        ANeighborGridActor* NeighborGridActor,
        FVector Origin,
        float Radius,
        float Height,
        const TArray<FSubjectHandle>& IgnoreSubjects,
        FFilter Filter,
        FSubjectHandle& Result
    );

    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (AutoCreateRefTerm = "IgnoreSubjects"))
    static void SectorExpandForSubject(
        ANeighborGridActor* NeighborGridActor,
        FVector Origin,
        float Radius,
        float Height,
        FVector Direction,
        float Angle,
        bool bCheckVisibility,
        const TArray<FSubjectHandle>& IgnoreSubjects,
        FFilter Filter,
        FSubjectHandle& Result
    );

    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (AutoCreateRefTerm = "IgnoreSubjects"))
    static FDmgResult ApplyDamageToSubjects(
        ABattleFrameBattleControl* BattleControl,
        TArray<FSubjectHandle> Subjects,
        const TArray<FSubjectHandle>& IgnoreSubjects,
        FSubjectHandle DmgInstigator,
        FVector HitFromLocation,
        FDmgSphere DmgSphere,
        FDebuff Debuff
    );

    UFUNCTION(BlueprintCallable, BlueprintPure,meta = (DisplayName = "Convert to SubjectHandles",CompactNodeTitle = "->",BlueprintAutocast),Category = "BattleFrame")
    static TArray<FSubjectHandle> ConvertTraceResultsToSubjectHandles(const TArray<FTraceResult>& TraceResults);

    UFUNCTION(BlueprintCallable, Category = "BattleFrame", meta = (DisplayName = "Sort Subjects By Distance", Keywords = "Sort Distance Subject"))
    static void SortSubjectsByDistance(UPARAM(ref) TArray<FTraceResult>& Results, const FVector& SortOrigin, ESortMode SortMode = ESortMode::NearToFar);

    static void SetSubTypeTraitByIndex(int32 Index, FSubjectRecord& SubjectRecord);
    static void SetSubTypeTraitByEnum(ESubType SubType, FSubjectRecord& SubjectRecord);
    static void IncludeSubTypeTraitByIndex(int32 Index, FFilter& Filter);
    static void CalculateThreadsCountAndBatchSize(int32 IterableNum, int32 MaxThreadsAllowed, int32 MinBatchSizeAllowed, int32& ThreadsCount, int32& BatchSize);
    static void SetAvoGroupTraitByIndex(int32 Index, FSubjectRecord& SubjectRecord);
    static void SetTeamTraitByIndex(int32 Index, FSubjectRecord& SubjectRecord);
    static void IncludeAvoGroupTraitByIndex(int32 Index, FFilter& Filter);
    static void ExcludeAvoGroupTraitByIndex(int32 Index, FFilter& Filter);
};

//-------------------------------Async Trace-------------------------------

UENUM(BlueprintType)
enum class EAsyncSortMode : uint8
{
    None UMETA(DisplayName = "None", ToolTip = "不排序"),
    NearToFar UMETA(DisplayName = "Near to Far", ToolTip = "从近到远"),
    FarToNear UMETA(DisplayName = "Far to Near", ToolTip = "从远到近")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncTraceOutput, const TArray<FTraceResult>&, Results);

UCLASS()
class BATTLEFRAME_API USphereSweepForSubjectsAsyncAction : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintAssignable)
    FAsyncTraceOutput Completed;

    TWeakObjectPtr<ANeighborGridActor> NeighborGridActor;
    FVector Start;
    FVector End;
    float Radius;
    EAsyncSortMode SortMode;
    FVector SortOrigin;
    int32 MaxCount;
    FFilter Filter;
    TArray<FSubjectHandle> IgnoreSubjects;

    TArray<FTraceResult> TempResults;
    TArray<FTraceResult> Results;

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm = "IgnoreSubjects"))
    static USphereSweepForSubjectsAsyncAction* SphereSweepForSubjectsAsync
    (
        const UObject* WorldContextObject, 
        ANeighborGridActor* NeighborGridActor, 
        const FVector Start, 
        const FVector End, 
        float Radius, 
        const EAsyncSortMode SortMode, 
        const FVector SortOrigin, 
        const int32 MaxCount, 
        const TArray<FSubjectHandle>& IgnoreSubjects, 
        const FFilter Filter
    );

    virtual void Activate() override;
};
