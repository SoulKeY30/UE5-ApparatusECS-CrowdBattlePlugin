/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#include "BattleFramePlayerCharacter.h"
#include "Traits/Hero.h"
#include "Traits/Health.h"
#include "Traits/Avoiding.h"
#include "Traits/Located.h"
#include "Traits/Collider.h"
#include "Traits/Statistics.h"

// Sets default values
ABattleFramePlayerCharacter::ABattleFramePlayerCharacter()
{
	Subjective = CreateDefaultSubobject<USubjectiveActorComponent>("Subjective");

	Subjective->SetTrait(FHero{});
	Subjective->SetTrait(FHealth{});
	Subjective->SetTrait(FCollider{});
	Subjective->SetTrait(FStatistics{});

	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABattleFramePlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// if missing, add these traits
	Subjective->ObtainTrait<FHealth>();
	Subjective->SetTrait(FLocated{ GetActorLocation() });

	const auto Handle = Subjective->GetHandle();
	const auto Collider = Subjective->ObtainTrait<FCollider>();
	Subjective->SetTrait(FAvoiding{ GetActorLocation(),Collider.Radius, Handle, Handle.CalcHash()});
}

// Called every frame
void ABattleFramePlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Subjective->GetTraitPtr<FLocated, EParadigm::Unsafe>()->Location = GetActorLocation();
}

// Called to bind functionality to input
void ABattleFramePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

