/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#pragma once

// C++
#include <utility>

// Unreal
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StreamableManager.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "HAL/PlatformMisc.h"

// Apparatus
#include "Mechanism.h"
#include "MechanicalGameModeBase.h"

// BattleFrame
#include "Traits/Debuff.h"
#include "Traits/DmgSphere.h"
#include "Traits/SubType.h"
#include "Traits/Animation.h"

#include "BattleFrameGameMode.generated.h"

// Forward Declearation
class UNeighborGridComponent;

USTRUCT(BlueprintType) struct FResult
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FSubjectHandle> DamagedSubjects;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<bool> IsCritical;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<bool> IsKill;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<float> DmgDealt;
};

UCLASS()
class BATTLEFRAME_API ABattleFrameGameMode
	: public AMechanicalGameModeBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Performance)
	int32 MaxThreadsAllowed = FMath::Clamp(FPlatformMisc::NumberOfWorkerThreadsToSpawn() - 1, 1, 20);

	int32 ThreadsCount = 1;
	int32 BatchSize = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Sound)
	int32 NumSoundsPerFrame = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Sound)
	float SoundVolume = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Game)
	bool bIsGameOver = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Statistics)
	int32 AgentCount = 0;

	static ABattleFrameGameMode* Instance;
	FStreamableManager StreamableManager;
	UWorld* CurrentWorld = nullptr;
	AMechanism* Mechanism = nullptr;
	UNeighborGridComponent* NeighborGrid = nullptr;
	TQueue<TSoftObjectPtr<USoundBase>, EQueueMode::Mpsc> SoundsToPlay;
	TQueue<float> VolumesToPlay;
<<<<<<< HEAD
	EFlagmarkBit ReloadFlowFieldFlag = EFlagmarkBit::R;
	EFlagmarkBit TracingFlag = EFlagmarkBit::T;
=======

>>>>>>> parent of 0f9a801 (Beta.2)

public:

	ABattleFrameGameMode()
	{
		PrimaryActorTick.bCanEverTick = true;
	}

	void BeginPlay() override;

	void EndPlay(const EEndPlayReason::Type EndPlayReason) override
	{
		if (Instance == this)
		{
			Instance = nullptr;
		}

		Super::EndPlay(EndPlayReason);
	}

	void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static ABattleFrameGameMode* GetInstance()
	{
		return Instance;
	}

	UFUNCTION(BlueprintCallable, Category = "Damage")
	FResult ApplyDamageToSubjects(
		const TArray<FSubjectHandle> Subjects,
		const TArray<FSubjectHandle> IgnoreSubjects,
		FSubjectHandle DmgInstigator,
		FVector HitFromLocation,
		const FDmgSphere DmgSphere,
		const FDebuff Debuff
	);

	std::pair<bool, float> ProcessCritDamage(float BaseDamage, float damageMult, float Probability);

	void QueueSound(TSoftObjectPtr<USoundBase> Sound);

	void QueueText(FSubjectHandle Subject, float Value, float Style, float Scale, float Radius, FVector Location);

	void QueueFx(FSubjectHandle Subject, FTransform Transform, ESubType SubType);

	static void CopyAnimData(FAnimation& Animation);

};