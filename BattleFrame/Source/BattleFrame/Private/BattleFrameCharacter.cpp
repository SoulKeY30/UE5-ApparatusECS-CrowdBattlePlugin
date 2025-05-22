/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#include "BattleFrameCharacter.h"
#include "Traits/Hero.h"
#include "Traits/Health.h"
#include "Traits/GridData.h"
#include "Traits/Located.h"
#include "Traits/Collider.h"
#include "Traits/Statistics.h"
#include "Traits/BindFlowField.h"
#include "Traits/IsSubjective.h"
#include "Traits/Activated.h"
#include "BattleFrameBattleControl.h"

// Sets default values
ABattleFrameCharacter::ABattleFrameCharacter()
{
	Subjective = CreateDefaultSubobject<UBFSubjectiveActorComponent>("Subjective");
	Subjective->SetTrait(FHero{});

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



