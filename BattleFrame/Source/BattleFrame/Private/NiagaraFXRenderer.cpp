/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#include "NiagaraFXRenderer.h"
#include "Traits/Directed.h"
#include "Traits/Scaled.h"
#include "Traits/Rendering.h"
#include "Traits/SpawningFx.h"
#include "Traits/Located.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"


ANiagaraFXRenderer::ANiagaraFXRenderer()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Create and setup the Niagara component
    NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
    NiagaraComponent->SetupAttachment(RootComponent);
    NiagaraComponent->SetAutoActivate(false);
}

// Called when the game starts or when spawned
void ANiagaraFXRenderer::BeginPlay()
{
    Super::BeginPlay();

    if (NiagaraAsset)
    {
        // Set the asset and activate the component
        NiagaraComponent->SetAsset(NiagaraAsset);
        NiagaraComponent->Activate();
    }
}

void ANiagaraFXRenderer::Tick(float DeltaTime)
{
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("NiagaraFxRendererTick");

    Super::Tick(DeltaTime);

    const auto Mechanism = UMachine::ObtainMechanism(GetWorld());

    switch (Mode)
    {
        case EFxMode::InPlace:
        {
            TArray<FVector> NewLocationArray;
            TArray<FVector> NewDirectionArray;
            TArray<FVector> NewScaleArray;

            FFilter Filter1 = FFilter::Make<FLocated, FDirected, FScaled>();
            Filter1 += TraitType;
            Filter1 += SubType;

            Mechanism->Operate<FUnsafeChain>(Filter1,
                [&](FSubjectHandle Subject,
                    const FLocated& Located,
                    const FDirected& Directed,
                    const FScaled& Scaled)
                {
                    NewLocationArray.Add(Located.Location);
                    NewDirectionArray.Add(Directed.Direction);
                    NewScaleArray.Add(Scaled.Factors);

                    Subject.Despawn();
                });

            if (NiagaraComponent && NiagaraComponent->IsActive())
            {
                UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
                    NiagaraComponent,
                    FName("LocationArray"),
                    NewLocationArray
                );

                UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
                    NiagaraComponent,
                    FName("DirectionArray"),
                    NewDirectionArray
                );

                UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
                    NiagaraComponent,
                    FName("ScaleArray"),
                    NewScaleArray
                );
            }
            break;
        }

        case EFxMode::Attached:
        {
            FFilter Filter2 = FFilter::Make<FLocated, FDirected, FScaled>().Exclude<FRendering>();
            Filter2 += TraitType;
            Filter2 += SubType;

            Mechanism->Operate<FUnsafeChain>(
                Filter2,
                [&](FSubjectHandle Subject,
                    const FLocated& Located,
                    const FDirected& Directed,
                    const FScaled& Scaled)
                {
                    FQuat Rotation{ FQuat::Identity };
                    Rotation = Directed.Direction.Rotation().Quaternion();
                    FVector Scale = Scaled.Factors;

                    FTransform SubjectTransform(Rotation, Located.Location, Scale);

                    int32 NewInstanceId;
                    FRendering Rendering;

                    if (FreeTransforms.Num())
                    {
                        // Pick an index which is no longer in cooldown
                        while (FreeTransforms.Num() && CoolDowns[FreeTransforms.Top()] > 0)
                        {
                            FreeTransforms.Pop();
                        }

                        if (FreeTransforms.Num())
                        {
                            NewInstanceId = FreeTransforms.Pop(); // Reuse an existing instance ID
                            Transforms[NewInstanceId] = SubjectTransform; // Update the corresponding transform

                            LocationArray[NewInstanceId] = SubjectTransform.GetLocation();
                            OrientationArray[NewInstanceId] = SubjectTransform.GetRotation();
                            ScaleArray[NewInstanceId] = SubjectTransform.GetScale3D();
                        }
                        else
                        {
                            Transforms.Add(SubjectTransform);
                            NewInstanceId = Transforms.Num() - 1;

                            LocationArray.Add(SubjectTransform.GetLocation());
                            OrientationArray.Add(SubjectTransform.GetRotation());
                            ScaleArray.Add(SubjectTransform.GetScale3D());

                            CoolDowns.Add(0.0f);
                            LocationEventArray.Add(true);
                        }
                    }
                    else
                    {
                        Transforms.Add(SubjectTransform);
                        NewInstanceId = Transforms.Num() - 1;

                        LocationArray.Add(SubjectTransform.GetLocation());
                        OrientationArray.Add(SubjectTransform.GetRotation());
                        ScaleArray.Add(SubjectTransform.GetScale3D());

                        CoolDowns.Add(0.0f);
                        LocationEventArray.Add(true);
                    }

                    Rendering.InstanceId = NewInstanceId;

                    Subject.SetTrait(Rendering);
                });

            ValidTransforms.Reset();

            FFilter Filter3 = FFilter::Make<FLocated, FDirected, FScaled, FRendering>();
            Filter3 += TraitType;
            Filter3 += SubType;

            Mechanism->Operate<FUnsafeChain>(
                Filter3,
                [&](FSubjectHandle Subject,
                    const FLocated& Located,
                    const FDirected& Directed,
                    const FScaled& Scaled,
                    FRendering& Rendering)
                {
                    FQuat Rotation{ FQuat::Identity };
                    Rotation = Directed.Direction.Rotation().Quaternion();
                    FVector Scale = Scaled.Factors;

                    FTransform SubjectTransform(Rotation, Located.Location, Scale);

                    int32 InstanceId = Rendering.InstanceId;

                    ValidTransforms[InstanceId] = true;
                    Transforms[InstanceId] = SubjectTransform;

                    // Update arrays
                    LocationArray[InstanceId] = SubjectTransform.GetLocation();
                    OrientationArray[InstanceId] = SubjectTransform.GetRotation();
                    ScaleArray[InstanceId] = SubjectTransform.GetScale3D();
                });

            FreeTransforms.Reset();

            for (int32 i = ValidTransforms.IndexOf(false);
                i < Transforms.Num();
                i = ValidTransforms.IndexOf(false, i + 1))
            {
                FreeTransforms.Add(i);

                // Set the cooldown timer to 2 seconds for the released transforms.
                CoolDowns[i] = 2.0f;

                LocationEventArray[i] = false;

                Transforms[i].SetScale3D(FVector::ZeroVector);
                ScaleArray[i] = FVector::ZeroVector;

                Transforms[i].SetLocation(FVector(0, 0, 100000));
                LocationArray[i] = FVector(0, 0, 100000);
            }

            // Update CoolDowns array and LocationEvent array every frame by subtracting DeltaTime.
            for (int32 i = 0; i < CoolDowns.Num(); ++i)
            {
                CoolDowns[i] -= DeltaTime;

                if (CoolDowns[i] <= 0.0f)
                {
                    LocationEventArray[i] = true;
                }
            }

            if (NiagaraComponent && NiagaraComponent->IsActive())
            {
                // sync niagara
                UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
                    NiagaraComponent,
                    FName("LocationArray"),
                    LocationArray
                );

                UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayQuat(
                    NiagaraComponent,
                    FName("OrientationArray"),
                    OrientationArray
                );

                UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
                    NiagaraComponent,
                    FName("ScaleArray"),
                    ScaleArray
                );

                UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayBool(
                    NiagaraComponent,
                    FName("LocationEventArray"),
                    LocationEventArray
                );
            }

            break;
        }
    }
}
