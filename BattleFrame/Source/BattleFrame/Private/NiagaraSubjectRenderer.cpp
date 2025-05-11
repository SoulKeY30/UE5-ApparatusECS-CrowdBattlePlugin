/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#include "NiagaraSubjectRenderer.h"
#include "BattleFrameFunctionLibraryRT.h"
#include "HAL/Platform.h"

// Sets default values
ANiagaraSubjectRenderer::ANiagaraSubjectRenderer()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	AddTickPrerequisiteActor(ABattleFrameBattleControl::GetInstance());
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
	}
}

void ANiagaraSubjectRenderer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (FSubjectHandle Subject : SpawnedRendererSubjects)
	{
		Subject->DespawnDeferred();
	}

	for (UNiagaraComponent* System : SpawnedNiagaraSystems)
	{
		if (System)
		{
			System->DestroyComponent();
		}
	}

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

	if (TickEnabled)
	{
		if (!Initialized)
		{
			if (SubType.Index >= 0)
			{
				ActivateRenderer();
				Initialized = true;
			}
		}
		else
		{
			Register();

			// remove if no agent to render.
			if (IdleCheckTimer -= SafeDeltaTime <= 0)
			{
				if (IdleCheck())
				{
					Destroy();
				}
				else
				{
					IdleCheckTimer = 1;
				}
			}
		}
	}
}

void ANiagaraSubjectRenderer::ActivateRenderer()
{
	Mechanism = UMachine::ObtainMechanism(GetWorld());
	if (Mechanism == nullptr) return;

	for (int i = 0; i < NumRenderBatch; ++i)
	{
		FSubjectHandle RendererHandle = Mechanism->SpawnSubject(FRenderBatchData{});
		SpawnedRendererSubjects.Add(RendererHandle);
		
		FRenderBatchData* Data = RendererHandle.GetTraitPtr<FRenderBatchData, EParadigm::Unsafe>();

		Data->Scale = Scale;
		Data->OffsetLocation = OffsetLocation;
		Data->OffsetRotation = OffsetRotation;

		auto System = UNiagaraFunctionLibrary::SpawnSystemAtLocation
		(
			GetWorld(),
			NiagaraSystemAsset,
			GetActorLocation() + FVector(i * 250.f, 0, 0),
			FRotator::ZeroRotator, // rotation
			FVector(1), // scale
			false, // auto destroy
			true, // auto activate
			ENCPoolMethod::None,
			true
		);

		System->SetVariableStaticMesh(TEXT("StaticMesh"), StaticMesh);

		Data->SpawnedNiagaraSystem = System;
		SpawnedNiagaraSystems.Add(System);		
	}

	isActive = true;
}

void ANiagaraSubjectRenderer::Register()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("RenderRegister");

	if (isActive)
	{
		Mechanism = UMachine::ObtainMechanism(GetWorld());
		if (Mechanism == nullptr) return;

		FFilter Filter = FFilter::Make<FAgent, FLocated, FDirected, FScaled, FAnimation, FHealthBar, FAnimation, FActivated>().Exclude<FRendering>();
		UBattleFrameFunctionLibraryRT::IncludeSubTypeTraitByIndex(SubType.Index, Filter);

		Mechanism->Operate<FUnsafeChain>(Filter,
			[&](FSubjectHandle Subject,
				const FLocated& Located,
				const FDirected& Directed,
				const FScaled& Scaled,
				FHealthBar HealthBar,
				FAnimation Anim)
			{
				FQuat Rotation{ FQuat::Identity };
				Rotation = Directed.Direction.Rotation().Quaternion();

				FVector FinalScale(Scale);
				FinalScale *= Scaled.renderFactors;

				float Radius = 0.0f;

				if (Subject.HasTrait<FCollider>())
				{
					const auto Collider = Subject.GetTrait<FCollider>();
					Radius = Collider.Radius;
				}

				FTransform SubjectTransform(
					Rotation * OffsetRotation.Quaternion(),
					Located.Location + OffsetLocation - FVector(0, 0, Radius),
					FinalScale);

				int32 NewInstanceId;
				FSubjectHandle RendererSubject;
				FRendering Rendering;

				int32 Selector = BatchSelector % NumRenderBatch;
				Rendering.Renderer = SpawnedRendererSubjects[Selector];
				FRenderBatchData* Data = Rendering.Renderer.GetTraitPtr<FRenderBatchData, EParadigm::Unsafe>();
				if (Data == nullptr) { return; }

				// Lock the critical section
				Data->Lock();

				// Check if FreeTransforms has any members
				if (Data->FreeTransforms.Num())
				{
					NewInstanceId = Data->FreeTransforms.Pop(); // Reuse an existing instance ID
					Data->Transforms[NewInstanceId] = SubjectTransform; // Update the corresponding transform

					Data->LocationArray[NewInstanceId] = SubjectTransform.GetLocation();
					Data->OrientationArray[NewInstanceId] = SubjectTransform.GetRotation();
					Data->ScaleArray[NewInstanceId] = SubjectTransform.GetScale3D();

					Data->Anim_Index0_Index1_PauseTime0_PauseTime1_Array[NewInstanceId] = FVector4(Anim.AnimIndex0, Anim.AnimIndex1, Anim.AnimPauseTime0, Anim.AnimPauseTime1);
					Data->Anim_TimeStamp0_TimeStamp1_PlayRate0_Playrate1_Array[NewInstanceId] = FVector4(GetGameTimeSinceCreation(), GetGameTimeSinceCreation(), 1, 1);

					Data->Anim_Lerp_Array[NewInstanceId] = 0;

					Data->Mat_HitGlow_Freeze_Burn_Dissolve_Array[NewInstanceId] = FVector4(0, 0, 0, 1);
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

					Data->Anim_Index0_Index1_PauseTime0_PauseTime1_Array.Add(FVector4(Anim.AnimIndex0, Anim.AnimIndex1, Anim.AnimPauseTime0, Anim.AnimPauseTime1));
					Data->Anim_TimeStamp0_TimeStamp1_PlayRate0_Playrate1_Array.Add(FVector4(GetGameTimeSinceCreation(), GetGameTimeSinceCreation(), 1, 1));

					Data->Anim_Lerp_Array.Add(0);

					Data->Mat_HitGlow_Freeze_Burn_Dissolve_Array.Add(FVector4(0, 0, 0, 1));
					Data->HealthBar_Opacity_CurrentRatio_TargetRatio_Array.Add(FVector(HealthBar.Opacity, HealthBar.CurrentRatio, HealthBar.TargetRatio));

					Data->InsidePool_Array.Add(false);
				}

				// Unlock the critical section
				Data->Unlock();

				Rendering.InstanceId = NewInstanceId;
				Subject.SetTrait(Rendering);
				BatchSelector++;
			}
		);
	}
}

bool ANiagaraSubjectRenderer::IdleCheck()
{
	bool isIdle = true;

	for (int i = 0; i < NumRenderBatch; ++i)
	{
		if (isIdle)
		{
			FRenderBatchData thisRenderBatchData = SpawnedRendererSubjects[i].GetTrait<FRenderBatchData>();
			bool thisIdle = thisRenderBatchData.FreeTransforms.Num() == thisRenderBatchData.Transforms.Num();
			
			if (!thisIdle)
			{
				isIdle = false;
				return isIdle;
			}
		}
	}

	return isIdle;
}
