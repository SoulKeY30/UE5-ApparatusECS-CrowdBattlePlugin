/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#pragma once

#include "Modules/ModuleManager.h"


// Forward declarations:
class ANeighborGridActor;

/**
 * The main NeighborGrid editor module.
 */
class BATTLEFRAMEEDITOR_API FBattleFrameEditorModule
  : public IModuleInterface
{
	friend class ANeighborGridActor;

  public:

#pragma region IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
#pragma endregion IModuleInterface
};
