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

#include "Traits/SubType.h"
#include "Traits/Projectile.h"
#include "Traits/DmgSphere.h"
#include "Traits/Moving.h"
#include "Traits/BeingSpawned.h"
#include "Traits/Animation.h"
#include "Traits/RenderBatch.h"
#include "Traits/RenderBatchData.h"
#include "Traits/Aging.h"

#include "Traits/Located.h" 
#include "Traits/Rendering.h"
#include "Traits/Directed.h"
#include "Traits/Rotated.h"
#include "Traits/Scaled.h"
#include "Traits/Collider.h"
#include "Traits/HealthBar.h"
#include "Traits/Agent.h"

#include "Stats/Stats.h"
#include "Kismet/GameplayStatics.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "BattleFrameGameMode.h"


// Debugging
#include "DrawDebugHelpers.h"
#include <Engine/StaticMesh.h>

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
    
    // Public Methods
    void Register();
    void ActivateRenderer();

    UFUNCTION(BlueprintCallable)
    bool IdleCheck();

    // Trait Types
    UPROPERTY(BlueprintReadWrite, EditAnywhere, NoClear, Category = "Settings")
    FAgent Agent;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, NoClear, Category = "Settings")
    FSubType SubType = FSubType{ -1 };

    // Niagara Effects
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
    UNiagaraSystem* NiagaraSystemAsset;

    // Rendering Settings
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
    FVector Scale = {1.0f, 1.0f, 1.0f};

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
    FVector OffsetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
    FRotator OffsetRotation = FRotator(0,0,0);

    bool Initialized = false;
    bool TickEnabled = false;
    bool isActive = false;

    // Performance Settings
    //UPROPERTY(EditAnywhere, Category = Performance)
    //int32 CurrentThreadsCount = 8;

    //UPROPERTY(EditAnywhere, Category = Performance)
    //int32 CurrentBatchSize = 1;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Performance)
    int32 NumRenderBatch = 4;

    //UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Performance)
    int32 RAMReserve = 5000;

    UNiagaraComponent* SpawnedNiagaraSystem;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CachedVars")
    TArray<UNiagaraComponent*> SpawnedNiagaraSystems;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CachedVars")
    TArray<FSubjectHandle> SpawnedRendererSubjects;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CachedVars")
    UStaticMesh* StaticMesh;

    int64 BatchSelector = 0;


};
