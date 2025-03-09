#pragma once

#include "CoreMinimal.h"
#include "Tower.generated.h"


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTower
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 TowerType = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UStaticMeshComponent* towerMesh = nullptr;
};
