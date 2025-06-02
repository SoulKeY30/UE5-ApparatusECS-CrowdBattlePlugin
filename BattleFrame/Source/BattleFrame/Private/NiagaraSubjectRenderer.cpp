/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#include "NiagaraSubjectRenderer.h"
#include "BattleFrameFunctionLibraryRT.h"
#include "HAL/Platform.h"
#include "Traits/SubType.h"
#include "Traits/Animation.h"
#include "Traits/RenderBatchData.h"
#include "Traits/Located.h" 
#include "Traits/Rendering.h"
#include "Traits/Directed.h"
#include "Traits/Scaled.h"
#include "Traits/Collider.h"
#include "Traits/HealthBar.h"
#include "Traits/Agent.h"
#include "BattleFrameBattleControl.h"


// Sets default values
ANiagaraSubjectRenderer::ANiagaraSubjectRenderer()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

// Called when the game starts or when spawned
void ANiagaraSubjectRenderer::BeginPlay()
{
	Super::BeginPlay();

	CurrentWorld = GetWorld();

	if (CurrentWorld)
	{
		Mechanism = UMachine::ObtainMechanism(CurrentWorld);
		BattleControl = Cast<ABattleFrameBattleControl>(UGameplayStatics::GetActorOfClass(CurrentWorld, ABattleFrameBattleControl::StaticClass()));

		if (Mechanism && BattleControl && SubType.Index >= 0)
		{
			Initialized = true;
		}
	}
}

void ANiagaraSubjectRenderer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	CurrentWorld = GetWorld();

	if (CurrentWorld)
	{
		BattleControl = Cast<ABattleFrameBattleControl>(UGameplayStatics::GetActorOfClass(CurrentWorld, ABattleFrameBattleControl::StaticClass()));

		if (BattleControl)
		{
			BattleControl->ExistingRenderers.Remove(SubType.Index);
		}
	}
}

// Called every frame
void ANiagaraSubjectRenderer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float SafeDeltaTime = FMath::Clamp(DeltaTime, 0, 0.0333f);

	if (Initialized)
	{
		Register();
		IdleCheck();
	}
}

void ANiagaraSubjectRenderer::Register()
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("RenderRegister");

	FFilter Filter = FFilter::Make<FAgent, FCollider, FLocated, FDirected, FScaled, FAnimation, FHealthBar, FAnimation, FActivated>().Exclude<FRendering>();
	UBattleFrameFunctionLibraryRT::IncludeSubTypeTraitByIndex(SubType.Index, Filter);

	Mechanism->Operate<FUnsafeChain>(Filter,
		[&](const FSubjectHandle Subject,
			const FCollider& Collider,
			const FLocated& Located,
			const FDirected& Directed,
			const FScaled& Scaled,
			const FHealthBar& HealthBar,
			const FAnimation& Anim)
		{
			FQuat Rotation{ FQuat::Identity };
			Rotation = Directed.Direction.Rotation().Quaternion();

			FVector FinalScale(Scale);
			FinalScale *= Scaled.renderFactors;

			float Radius = Collider.Radius;

			FTransform SubjectTransform(Rotation * OffsetRotation.Quaternion(),Located.Location + OffsetLocation - FVector(0, 0, Radius),FinalScale);

			FSubjectHandle RenderBatch = FSubjectHandle();
			FRenderBatchData* Data = nullptr;

			// Get the first render batch that has a free trans slot
			for (const FSubjectHandle Renderer : SpawnedRenderBatches)
			{
				FRenderBatchData* CurrentData = Renderer.GetTraitPtr<FRenderBatchData, EParadigm::Unsafe>();

				if (CurrentData->FreeTransforms.Num() > 0 || CurrentData->Transforms.Num() < RenderBatchSize)
				{
					RenderBatch = Renderer;
					Data = Renderer.GetTraitPtr<FRenderBatchData, EParadigm::Unsafe>();
					break;
				}
			}

			if (!Data)// all current batches are full
			{
				RenderBatch = AddRenderBatch();// add a new batch
				Data = RenderBatch.GetTraitPtr<FRenderBatchData, EParadigm::Unsafe>();
			}

			int32 NewInstanceId;

			// Check if FreeTransforms has any members
			if (!Data->FreeTransforms.IsEmpty())
			{
				NewInstanceId = Data->FreeTransforms.Pop(); // Reuse an existing instance ID
				Data->Transforms[NewInstanceId] = SubjectTransform; // Update the corresponding transform

				Data->LocationArray[NewInstanceId] = SubjectTransform.GetLocation();
				Data->OrientationArray[NewInstanceId] = SubjectTransform.GetRotation();
				Data->ScaleArray[NewInstanceId] = SubjectTransform.GetScale3D();

				Data->Anim_Lerp_Array[NewInstanceId] = 0;

				Data->Anim_Index0_Index1_PauseTime0_PauseTime1_Array[NewInstanceId] = FVector4(Anim.AnimIndex0, Anim.AnimIndex1, Anim.AnimPauseTime0, Anim.AnimPauseTime1);
				Data->Anim_TimeStamp0_TimeStamp1_PlayRate0_Playrate1_Array[NewInstanceId] = FVector4(GetGameTimeSinceCreation(), GetGameTimeSinceCreation(), 1, 1);

				Data->Mat_Dissolve_HitGlow_Team_Fire_Array[NewInstanceId] = FVector4(1, 0, 0, 0);
				Data->Mat_Ice_Poison_Array[NewInstanceId] = FVector4(0, 0, 0, 0);

				Data->HealthBar_Opacity_CurrentRatio_TargetRatio_Array[NewInstanceId] = FVector(HealthBar.Opacity, HealthBar.CurrentRatio, HealthBar.TargetRatio);

				Data->InsidePool_Array[NewInstanceId] = false;
			}
			else
			{
				// Add new instance and get its ID
				Data->Transforms.Add(SubjectTransform);
				NewInstanceId = Data->Transforms.Num() - 1;

				// SubjectTransform
				Data->LocationArray.Add(SubjectTransform.GetLocation());
				Data->OrientationArray.Add(SubjectTransform.GetRotation());
				Data->ScaleArray.Add(SubjectTransform.GetScale3D());

				Data->Anim_Lerp_Array.Add(0);

				Data->Anim_Index0_Index1_PauseTime0_PauseTime1_Array.Add(FVector4(Anim.AnimIndex0, Anim.AnimIndex1, Anim.AnimPauseTime0, Anim.AnimPauseTime1));
				Data->Anim_TimeStamp0_TimeStamp1_PlayRate0_Playrate1_Array.Add(FVector4(GetGameTimeSinceCreation(), GetGameTimeSinceCreation(), 1, 1));

				Data->Mat_Dissolve_HitGlow_Team_Fire_Array.Add(FVector4(1, 0, 0, 0));
				Data->Mat_Ice_Poison_Array.Add(FVector4(0, 0, 0, 0));

				Data->HealthBar_Opacity_CurrentRatio_TargetRatio_Array.Add(FVector(HealthBar.Opacity, HealthBar.CurrentRatio, HealthBar.TargetRatio));

				Data->InsidePool_Array.Add(false);
			}

			Subject.SetTrait(FRendering{ NewInstanceId, RenderBatch });
		}
	);
}

void ANiagaraSubjectRenderer::IdleCheck()
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("IdleCheck");
	TArray<FSubjectHandle> CachedSpawnedRenderBatches = SpawnedRenderBatches;

	for (const FSubjectHandle RenderBatch : CachedSpawnedRenderBatches)
	{
		FRenderBatchData* CurrentData = RenderBatch.GetTraitPtr<FRenderBatchData, EParadigm::Unsafe>();

		if (CurrentData->FreeTransforms.Num() == CurrentData->Transforms.Num())// current render batch is completely empty
		{
			RemoveRenderBatch(RenderBatch);
		}
	}

	if (SpawnedRenderBatches.IsEmpty())
	{
		this->Destroy();// remove this renderer
	}
}

FSubjectHandle ANiagaraSubjectRenderer::AddRenderBatch()
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("AddRenderBatch");
	FSubjectHandle RenderBatch = Mechanism->SpawnSubject(FRenderBatchData());
	SpawnedRenderBatches.Add(RenderBatch);

	FRenderBatchData* NewData = RenderBatch.GetTraitPtr<FRenderBatchData, EParadigm::Unsafe>();

	NewData->Scale = Scale;
	NewData->OffsetLocation = OffsetLocation;
	NewData->OffsetRotation = OffsetRotation;

	auto System = UNiagaraFunctionLibrary::SpawnSystemAtLocation
	(
		GetWorld(),
		NiagaraSystemAsset,
		GetActorLocation(),
		FRotator::ZeroRotator, // rotation
		FVector(1), // scale
		false, // auto destroy
		true, // auto activate
		ENCPoolMethod::None,
		true
	);

	System->SetVariableStaticMesh(TEXT("StaticMesh"), StaticMeshAsset);

	NewData->SpawnedNiagaraSystem = System;

	return RenderBatch;
}

void ANiagaraSubjectRenderer::RemoveRenderBatch(FSubjectHandle RenderBatch)
{
	//TRACE_CPUPROFILER_EVENT_SCOPE_STR("RemoveRenderBatch");
	FRenderBatchData* RenderBatchData = RenderBatch.GetTraitPtr<FRenderBatchData, EParadigm::Unsafe>();
	RenderBatchData->SpawnedNiagaraSystem->DestroyComponent();
	SpawnedRenderBatches.Remove(RenderBatch);
	RenderBatch->Despawn();
}