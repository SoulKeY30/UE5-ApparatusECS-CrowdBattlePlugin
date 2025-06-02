/*
 * APPARATIST
 * Created: 2023-02-02 16:26:26
 * Author: Vladislav Dmitrievich Turbanov (vladislav@turbanov.ru)
 * Community forums: https://talk.turbanov.ru
 * Copyright 2019 - 2023, SP Vladislav Dmitrievich Turbanov
 * Made in Russia, Moscow City, Chekhov City
 */

 /*
  * BattleFrame
  * Refactor: 2025
  * Author: Leroy Works
  */

#include "NeighborGridComponentVisualizer.h"

#include "NeighborGridComponent.h"
#include "SceneManagement.h"


void
FNeighborGridComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
    const auto NeighborGridComponent = Cast<const UNeighborGridComponent>(Component);

    if (!IsValid(NeighborGridComponent)) return;

    const auto Color = FLinearColor::Yellow;

    const auto Bounds = NeighborGridComponent->GetBounds();
    DrawWireBox(PDI, Bounds, Color, 1, /*DepthPriority=*/0, false);

    const auto CellSize = NeighborGridComponent->CellSize;
    const auto GridSize = NeighborGridComponent->GridSize;

    if (NeighborGridComponent->bDebugDrawCageCells)
    {
        // 修改循环范围，确保不超过实际的Cell数量
        for (int32 i = 0; i < GridSize.X; ++i)
        {
            for (int32 j = 0; j < GridSize.Y; ++j)
            {
                for (int32 k = 0; k < GridSize.Z; ++k)
                {
                    // 计算当前Cell的最小和最大坐标
                    const FVector CellMin = Bounds.Min + FVector(i, j, k) * CellSize;
                    const FVector CellMax = CellMin + CellSize;
                    const FBox CellBounds(CellMin, CellMax);

                    // 绘制当前Cell的WireBox
                    DrawWireBox(PDI, CellBounds, Color, 0, /*DepthPriority=*/0, false);
                }
            }
        }
    }
}
