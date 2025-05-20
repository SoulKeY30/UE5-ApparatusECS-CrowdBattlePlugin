/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#pragma once

// Unreal Engine Core
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "HAL/PlatformMisc.h"

// Niagara Systems
#include "NiagaraSystem.h" 
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h" 
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

// Apparatus and Custom Traits
#include "Machine.h"
#include "Stats/Stats.h"
#include "Kismet/GameplayStatics.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "BattleFrameBattleControl.h"

// Debugging
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"

// Generated UCLASS
#include "NiagaraSubjectRenderer.generated.h"


UCLASS()
class BATTLEFRAME_API ANiagaraSubjectRenderer : public AActor
{
    GENERATED_BODY()

public:	
    // Constructors and Lifecycle
    ANiagaraSubjectRenderer();

    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
    // Public Methods
    void Register();

    void IdleCheck();

    FSubjectHandle AddRenderBatch();

    void RemoveRenderBatch(FSubjectHandle RenderBatch);

    // Trait Types
    UPROPERTY(BlueprintReadWrite, EditAnywhere, NoClear, Category = "Settings")
    FAgent Agent;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, NoClear, Category = "Settings")
    FSubType SubType = FSubType{ -1 };

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
    int32 RenderBatchSize = 1000;

    // Rendering Settings
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
    FVector Scale = { 1.0f, 1.0f, 1.0f };

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
    FVector OffsetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
    FRotator OffsetRotation = FRotator(0, 0, 0);

    // Niagara Effects
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
    UNiagaraSystem* NiagaraSystemAsset;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
    UStaticMesh* StaticMeshAsset;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "CachedVars")
    TArray<FSubjectHandle> SpawnedRenderBatches;


    bool Initialized = false;
    UWorld* CurrentWorld = nullptr;
    AMechanism* Mechanism = nullptr;
    ABattleFrameBattleControl* BattleControl = nullptr;
    UNiagaraComponent* SpawnedNiagaraSystem;

};
