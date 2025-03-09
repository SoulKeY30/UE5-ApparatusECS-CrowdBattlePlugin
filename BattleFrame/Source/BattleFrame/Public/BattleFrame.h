/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FBattleFrameModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
