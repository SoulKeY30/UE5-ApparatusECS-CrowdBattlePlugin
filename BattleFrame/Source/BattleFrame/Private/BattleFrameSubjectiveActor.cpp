/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#include "BattleFrameSubjectiveActor.h"
#include "Traits/Prop.h"
#include "Traits/Health.h"
#include "Traits/Avoiding.h"
#include "Traits/Located.h"
#include "Traits/Collider.h"
#include "Traits/BindFlowField.h"

// Sets default values
ABattleFrameSubjectiveActor::ABattleFrameSubjectiveActor()
{
	Subjective = CreateDefaultSubobject<USubjectiveActorComponent>("Subjective");

	Subjective->SetTrait(FProp{});
	Subjective->SetTrait(FHealth{});
	Subjective->SetTrait(FCollider{});
	Subjective->SetTrait(FBindFlowField{});

	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABattleFrameSubjectiveActor::BeginPlay()
{
	Super::BeginPlay();

	// if missing, add these traits
	Subjective->ObtainTrait<FHealth>();
	Subjective->SetTrait(FLocated{ GetActorLocation() });

	const auto Handle = Subjective->GetHandle();
	const auto Collider = Subjective->ObtainTrait<FCollider>();
	Subjective->SetTrait(FAvoiding{ GetActorLocation(),Collider.Radius, Handle, Handle.CalcHash() });
}

// Called every frame
void ABattleFrameSubjectiveActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Subjective->GetTraitPtr<FLocated, EParadigm::Unsafe>()->Location = GetActorLocation();
}


