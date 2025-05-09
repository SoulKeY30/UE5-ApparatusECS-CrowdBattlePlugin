#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h" // Include the Niagara System header
#include "NiagaraComponent.h" // Include the Niagara Component header
#include "NiagaraFunctionLibrary.h" // Include the Niagara Function Library header
#include "SubjectHandle.h"
#include "SpawningFx.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSpawningFx
{
	GENERATED_BODY()

public:
	//float CoolDown = 2.f;

	FSubjectHandle AttachParentSubject = FSubjectHandle();

	UParticleSystemComponent* SpawnedCascadeSystem = nullptr;

	UNiagaraComponent* SpawnedNiagaraSystem = nullptr;

	bool bShouldDespawn = false;
};
