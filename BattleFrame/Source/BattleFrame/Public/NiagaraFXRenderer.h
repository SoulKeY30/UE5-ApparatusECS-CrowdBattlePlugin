/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h" // Include the Niagara System header
#include "NiagaraComponent.h" // Include the Niagara Component header
#include "Machine.h"
#include "HAL/PlatformMisc.h"

#include "NiagaraFXRenderer.generated.h"

UENUM(BlueprintType)
enum class EFxMode : uint8
{
	InPlace UMETA(DisplayName = "InPlace"),
	Attached UMETA(DisplayName = "Attached")
};

UCLASS()
class BATTLEFRAME_API ANiagaraFXRenderer : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANiagaraFXRenderer();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FxRenderer")
	EFxMode Mode = EFxMode::InPlace;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FxRenderer")
	UScriptStruct* TraitType = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FxRenderer")
	UScriptStruct* SubType = nullptr;

	// Niagara component that will handle the effects
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FxRenderer", meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* NiagaraComponent;

	// Variable to hold the Niagara System Asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FxRenderer")
	UNiagaraSystem* NiagaraAsset;

	TArray<FTransform> Transforms;
	FBitMask ValidTransforms;
	TArray<int32> FreeTransforms;
	TArray<float> CoolDowns;

	TArray<FVector> LocationArray;
	TArray<FQuat> OrientationArray;
	TArray<FVector> ScaleArray;
	TArray<bool> LocationEventArray;
};
