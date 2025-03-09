#pragma once
 
#include "CoreMinimal.h"
#include "NiagaraSystem.h" // Include the Niagara System header
#include "NiagaraComponent.h" // Include the Niagara Component header
#include "NiagaraFunctionLibrary.h" // Include the Niagara Function Library header
#include "FxHolder.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FFxHolder
{

	GENERATED_BODY()

public:

	UParticleSystemComponent* TrailFxComponent = nullptr;
};