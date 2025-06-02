#include "BFSubjectiveActorComponent.h"
#include "Traits/Health.h"
#include "Traits/GridData.h"
#include "Traits/Located.h"
#include "Traits/Collider.h"
#include "Traits/BindFlowField.h"
#include "Traits/Activated.h"
#include "Traits/IsSubjective.h"
#include "Traits/TemporalDamaging.h"

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
    UpdateTraits();
}

void UBFSubjectiveActorComponent::InitializeTraits(AActor* OwnerActor)
{
    if (!OwnerActor) return;

    // Ensure we have these traits
    ObtainTrait<FHealth>();
    ObtainTrait<FIsSubjective>();
    SetTrait(FLocated{ OwnerActor->GetActorLocation() });

    const auto SubjectHandle = GetHandle();
    const auto Collider = ObtainTrait<FCollider>();
    SetTrait(FGridData{ SubjectHandle.CalcHash(), FVector3f(OwnerActor->GetActorLocation()), Collider.Radius, SubjectHandle });

    SetTrait(FTemporalDamaging());
    SetTrait(FActivated());
}

void UBFSubjectiveActorComponent::UpdateTraits()
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor) return;

    auto Located = GetTraitPtr<FLocated, EParadigm::Unsafe>();
    if (Located)
    {
        Located->Location = OwnerActor->GetActorLocation();
    }
}
