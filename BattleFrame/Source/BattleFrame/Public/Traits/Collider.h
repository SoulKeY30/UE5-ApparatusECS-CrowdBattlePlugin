#pragma once

#include "CoreMinimal.h"
#include "Collider.generated.h"


USTRUCT(BlueprintType, Category = "NeighborGrid")
struct BATTLEFRAME_API FCollider
{
	GENERATED_BODY()

  public:

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	//bool EnableRVO = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NeighborGrid", Meta = (ClampMin = "0", ToolTip = "球体的半径"))
	float Radius = 50.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NeighborGrid", Meta = (ToolTip = "是否启用高质量模式"))
	bool bHightQuality = true;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NeighborGrid", Meta = (ClampMin = "0", ToolTip = "头部高度"))
	//float HeadRatio = 0.5f;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "NeighborGrid", Meta = (ToolTip = "是否绘制调试形状"))
	//bool bDrawDebugShape = false;



	/* Default constructor. */
	FCollider() {}

};
