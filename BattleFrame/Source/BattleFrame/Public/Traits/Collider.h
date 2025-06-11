#pragma once

#include "CoreMinimal.h"
#include "Collider.generated.h"


USTRUCT(BlueprintType, Category = "NeighborGrid")
struct BATTLEFRAME_API FCollider
{
	GENERATED_BODY()

  public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NeighborGrid", Meta = (ClampMin = "0", ToolTip = "球体的半径"))
	float Radius = 50.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NeighborGrid", Meta = (ToolTip = "注册Agent到所有相交的邻居网格内，适配巨型单位"))
	bool bHightQuality = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NeighborGrid", Meta = (ToolTip = "绘制Debug图形"))
	bool bDrawDebugShape = false;

};
