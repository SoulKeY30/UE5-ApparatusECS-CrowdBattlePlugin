/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#include "BattleFrameCharacter.h"


// Sets default values
ABattleFrameCharacter::ABattleFrameCharacter()
{
	PrimaryActorTick.bCanEverTick = true; 
}

// Called when the game starts or when spawned
void ABattleFrameCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABattleFrameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ABattleFrameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABattleFrameCharacter::ReceiveDamage_Implementation(const FDmgResult& DmgResult) 
{
 
}



