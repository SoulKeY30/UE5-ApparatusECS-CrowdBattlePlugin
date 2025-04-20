/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#include "RVOSphereObstacle.h"
#include "BattleFrameBattleControl.h"
// Sets default values
ARVOSphereObstacle::ARVOSphereObstacle()
{
	PrimaryActorTick.bCanEverTick = true;
	AddTickPrerequisiteActor(ABattleFrameBattleControl::GetInstance());
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ARVOSphereObstacle::BeginPlay()
{
	Super::BeginPlay();

	FSubjectRecord Template;
	Template.SetTrait(FLocated{});
	Template.SetTrait(FCollider{});
	Template.SetTrait(FSphereObstacle{});
	Template.SetTrait(FAvoidance{});

	FVector Location = SphereComponent->GetComponentLocation();
	float Radius = SphereComponent->GetScaledSphereRadius();

	Template.GetTraitRef<FLocated>().Location = Location;
	Template.GetTraitRef<FLocated>().preLocation = Location;
	Template.GetTraitRef<FCollider>().Radius = Radius;
	Template.GetTraitRef<FSphereObstacle>().bOverrideSpeedLimit = bOverrideSpeedLimit;
	Template.GetTraitRef<FSphereObstacle>().NewSpeedLimit = NewSpeedLimit;
	Template.GetTraitRef<FSphereObstacle>().bStatic = !bIsDynamicObstacle;
	Template.GetTraitRef<FAvoidance>().Radius = Radius;
	Template.GetTraitRef<FAvoidance>().Position = RVO::Vector2(Location.X, Location.Y);

	AMechanism* Mechanism = UMachine::ObtainMechanism(GetWorld());
	SubjectHandle = Mechanism->SpawnSubject(Template);

	SubjectHandle.SetTrait(FAvoiding{ Location, Radius, SubjectHandle, SubjectHandle.CalcHash()});

}

// Called every frame
void ARVOSphereObstacle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDynamicObstacle)
	{
		FVector Location = SphereComponent->GetComponentLocation();
		float Radius = SphereComponent->GetScaledSphereRadius();

		SubjectHandle.GetTraitRef<FLocated, EParadigm::Unsafe>().Location = Location;
		SubjectHandle.GetTraitRef<FLocated, EParadigm::Unsafe>().preLocation = Location;
		SubjectHandle.GetTraitRef<FCollider, EParadigm::Unsafe>().Radius = Radius;
		SubjectHandle.GetTraitRef<FSphereObstacle, EParadigm::Unsafe>().bOverrideSpeedLimit = bOverrideSpeedLimit;
		SubjectHandle.GetTraitRef<FSphereObstacle, EParadigm::Unsafe>().NewSpeedLimit = NewSpeedLimit;
		SubjectHandle.GetTraitRef<FAvoidance, EParadigm::Unsafe>().Radius = Radius;
		SubjectHandle.GetTraitRef<FAvoidance, EParadigm::Unsafe>().Position = RVO::Vector2(Location.X, Location.Y);
	}
}

// Called when the actor is being destroyed or the game is ending
void ARVOSphereObstacle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Check if the SubjectHandle is valid
	if (SubjectHandle.IsValid())
	{
		// Obtain the Mechanism instance
		AMechanism* Mechanism = UMachine::ObtainMechanism(GetWorld());
		if (Mechanism)
		{
			// Despawn the subject using the SubjectHandle
			SubjectHandle->Despawn();;
		}
	}
}

