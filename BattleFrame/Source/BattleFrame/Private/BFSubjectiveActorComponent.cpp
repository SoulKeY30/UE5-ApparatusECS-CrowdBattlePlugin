#include "BFSubjectiveActorComponent.h"
#include "Traits/Health.h"
#include "Traits/GridData.h"
#include "Traits/Located.h"
#include "Traits/Directed.h"
#include "Traits/Scaled.h"
#include "Traits/Collider.h"
#include "Traits/BindFlowField.h"
#include "Traits/Activated.h"
#include "Traits/IsSubjective.h"
#include "Traits/TemporalDamaging.h"
#include "Traits/Slowing.h"

UBFSubjectiveActorComponent::UBFSubjectiveActorComponent()
{
    // Set up basic traits in constructor
    SetTrait(FHealth());
    SetTrait(FCollider());
    SetTrait(FBindFlowField());

    // Enable ticking
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UBFSubjectiveActorComponent::BeginPlay()
{
    Super::BeginPlay();

    InitializeTraits(GetOwner());
}

void UBFSubjectiveActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AsyncTransformActorToSubject(GetOwner());
}

void UBFSubjectiveActorComponent::InitializeTraits(AActor* OwnerActor)
{
    if (!OwnerActor) return;

    // Ensure we have these traits
    if (!HasTrait<FHealth>())
    {
        SetTrait(FHealth());
    }

    if (!HasTrait<FCollider>())
    {
        SetTrait(FCollider());
    }

    SetTrait(FLocated{ OwnerActor->GetActorLocation() });
    SetTrait(FDirected{ OwnerActor->GetActorForwardVector().GetSafeNormal2D() });
    SetTrait(FScaled{ OwnerActor->GetActorScale3D() });

    SetTrait(FTemporalDamaging());
    SetTrait(FSlowing());

    const auto SubjectHandle = GetHandle();
    SetTrait(FGridData{ SubjectHandle.CalcHash(), FVector3f(GetTrait<FLocated>().Location), GetTrait<FCollider>().Radius, SubjectHandle });

    SetTrait(FIsSubjective());
    SetTrait(FActivated());
}

void UBFSubjectiveActorComponent::AsyncTransformActorToSubject(AActor* OwnerActor)
{
    if (!OwnerActor) return;

    auto Located = GetTraitPtr<FLocated, EParadigm::Unsafe>();

    if (Located)
    {
        Located->Location = OwnerActor->GetActorLocation();
    }

    auto Directed = GetTraitPtr<FDirected, EParadigm::Unsafe>();

    if (Directed)
    {
        Directed->Direction = OwnerActor->GetActorForwardVector().GetSafeNormal2D();
    }

    auto Scaled = GetTraitPtr<FScaled, EParadigm::Unsafe>();

    if (Scaled)
    {
        Scaled->Factors = OwnerActor->GetActorScale3D();
    }
}
