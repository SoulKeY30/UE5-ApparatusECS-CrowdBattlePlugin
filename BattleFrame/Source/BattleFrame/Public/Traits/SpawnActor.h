#pragma once

#include "CoreMinimal.h"
#include "SpawnActor.generated.h"

/**
 * The main projectile trait.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSpawnActor
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "出生时生成的贴花Actor类"))
	TSubclassOf<AActor> AppearDecalClass = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "出生贴花的偏移量"))
	FVector AppearDecalOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "投射物Actor类"))
	TSubclassOf<AActor> ProjectileClass = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "投射物的偏移量"))
	FVector ProjectileOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "危险警告Actor类"))
	TSubclassOf<AActor> DangerWarningClass = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "危险警告的偏移量"))
	FVector DangerWarningOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "死亡时生成的Actor类"))
	TSubclassOf<AActor> DeathSpawnClass = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "死亡生成Actor的缩放比例"))
	FVector DeathSpawnScale = FVector::OneVector;
};
