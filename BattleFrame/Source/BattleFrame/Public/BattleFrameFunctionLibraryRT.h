// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SubjectHandle.h"

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

    UFUNCTION(BlueprintCallable, Category = "Sorting", meta = (DisplayName = "Sort Subjects By Distance", Keywords = "Sort Distance Subject"))
    static void SortSubjectsByDistance(UPARAM(ref) TArray<FSubjectHandle>& Results, const FVector& SortOrigin, ESortMode SortMode = ESortMode::NearToFar);

    static void SetSubTypeTraitByIndex(int32 Index, FSubjectRecord& SubjectRecord);
    static void SetSubTypeTraitByEnum(ESubType SubType, FSubjectRecord& SubjectRecord);
    static void IncludeSubTypeTraitByIndex(int32 Index, FFilter& Filter);
    static void CalculateThreadsCountAndBatchSize(int32 IterableNum, int32& MaxThreadsAllowed, int32& ThreadsCount, int32& BatchSize);
<<<<<<< HEAD
    static void SetAvoGroupTraitByIndex(int32 Index, FSubjectRecord& SubjectRecord);
    static void SetTeamTraitByIndex(int32 Index, FSubjectRecord& SubjectRecord);
    static void IncludeAvoGroupTraitByIndex(int32 Index, FFilter& Filter);
    static void ExcludeAvoGroupTraitByIndex(int32 Index, FFilter& Filter);
=======
>>>>>>> parent of 0f9a801 (Beta.2)
};
