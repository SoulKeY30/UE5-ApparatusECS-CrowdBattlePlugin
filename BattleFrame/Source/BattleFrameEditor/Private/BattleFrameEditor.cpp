/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#include "BattleFrameEditor.h"

#include "UnrealEd.h"

#include "NeighborGridComponent.h"
#include "NeighborGridComponentVisualizer.h"

#define LOCTEXT_NAMESPACE "FBattleFrameEditorModule"

void FBattleFrameEditorModule::StartupModule()
{
	if (GUnrealEd != nullptr)
	{
		TSharedPtr<FComponentVisualizer> Visualizer = MakeShareable(new FNeighborGridComponentVisualizer());

		if (Visualizer.IsValid())
		{
			GUnrealEd->RegisterComponentVisualizer(UNeighborGridComponent::StaticClass()->GetFName(), Visualizer);
			Visualizer->OnRegister();
		}
	}
}

void FBattleFrameEditorModule::ShutdownModule()
{
	if (GUnrealEd != nullptr)
	{
		GUnrealEd->UnregisterComponentVisualizer(UNeighborGridComponent::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBattleFrameEditorModule, BattleFrameEditor)
