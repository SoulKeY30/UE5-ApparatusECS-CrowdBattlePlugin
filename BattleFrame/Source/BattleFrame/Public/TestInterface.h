// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BattleFrameStructs.h"
#include "TestInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTestInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class BATTLEFRAME_API ITestInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// 接受伤害时触发
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceiveDamage(const FDmgResult& DmgResult);
};
