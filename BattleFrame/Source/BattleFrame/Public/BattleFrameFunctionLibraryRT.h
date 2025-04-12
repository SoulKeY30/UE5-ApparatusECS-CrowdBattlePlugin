// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SubjectHandle.h"
#include "Traits/SubType.h"
#include "Traits/TraceResult.h"

#include "BattleFrameFunctionLibraryRT.generated.h"

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

    UFUNCTION(BlueprintCallable, BlueprintPure,meta = (DisplayName = "Convert to SubjectHandles",CompactNodeTitle = "->",BlueprintAutocast),Category = "BattleFrame|Utilities")
    static TArray<FSubjectHandle> ConvertTraceResultsToSubjectHandles(const TArray<FTraceResult>& TraceResults);

    UFUNCTION(BlueprintCallable, Category = "Sorting", meta = (DisplayName = "Sort Subjects By Distance", Keywords = "Sort Distance Subject"))
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
