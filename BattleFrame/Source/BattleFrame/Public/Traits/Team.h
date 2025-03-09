#pragma once
#include "CoreMinimal.h"
#include "Team.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTeam0
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTeam1
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTeam2
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTeam3
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTeam4
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTeam5
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTeam6
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTeam7
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTeam8
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTeam9
{
    GENERATED_BODY()
};


UENUM(BlueprintType)
enum class ETeam : uint8
{
    None UMETA(DisplayName = "None"),
    Team0 UMETA(DisplayName = "Team0"),
    Team1 UMETA(DisplayName = "Team1"),
    Team2 UMETA(DisplayName = "Team2"),
    Team3 UMETA(DisplayName = "Team3"),
    Team4 UMETA(DisplayName = "Team4"),
    Team5 UMETA(DisplayName = "Team5"),
    Team6 UMETA(DisplayName = "Team6"),
    Team7 UMETA(DisplayName = "Team7"),
    Team8 UMETA(DisplayName = "Team8"),
    Team9 UMETA(DisplayName = "Team9")
};