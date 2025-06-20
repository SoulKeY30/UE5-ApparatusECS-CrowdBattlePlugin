/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Machine.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector2D.h"
#include "AgentConfigDataAsset.h"
#include "BattleFrameStructs.h"
#include "AgentSpawner.generated.h"

class ABattleFrameBattleControl;

UCLASS()
class BATTLEFRAME_API AAgentSpawner : public AActor
{
	GENERATED_BODY()

protected:

	void BeginPlay() override;

public:

	AAgentSpawner();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<TSoftObjectPtr<UAgentConfigDataAsset>> AgentConfigAssets;

	UWorld* CurrentWorld = nullptr;
	AMechanism* Mechanism = nullptr;
	ABattleFrameBattleControl* BattleControl = nullptr;

	EFlagmarkBit RegisterMultipleFlag = EFlagmarkBit::M;

	UFUNCTION(BlueprintCallable, Category = "BattleFrame | AgentSpawner", meta = (DisplayName = "Spawn Agents By Config Index"))
	TArray<FSubjectHandle> SpawnAgentsRectangular
	(
		bool bAutoActivate = true,
		int32 ConfigIndex = 0,
		int32 Quantity = 1,
		int32 Team = 0,
		FVector Origin = FVector::ZeroVector,
		FVector2D Region = FVector2D::ZeroVector,
		FVector2D LaunchVelocity = FVector2D::ZeroVector,
		EInitialDirection InitialDirection = EInitialDirection::FacePlayer,
		FVector2D CustomDirection = FVector2D(1, 0),
		FSpawnerMult Multipliers = FSpawnerMult()
	);

	TArray<FSubjectHandle> SpawnAgentsByConfigRectangular
	(
		const bool bAutoActivate = true,
		const TSoftObjectPtr<UAgentConfigDataAsset> DataAsset = nullptr,
		const int32 Quantity = 1,
		const int32 Team = 0,
		const FVector& Origin = FVector::ZeroVector,
		const FVector2D& Region = FVector2D::ZeroVector,
		const FVector2D& LaunchVelocity = FVector2D::ZeroVector,
		const EInitialDirection InitialDirection = EInitialDirection::FacePlayer,
		const FVector2D& CustomDirection = FVector2D(1, 0),
		const FSpawnerMult& Multipliers = FSpawnerMult()
	);

	UFUNCTION(BlueprintCallable, Category = "BattleFrame | AgentSpawner", meta = (DisplayName = "Initialize Agent"))
	void ActivateAgent(FSubjectHandle Agent);

	UFUNCTION(BlueprintCallable, Category = "BattleFrame | AgentSpawner", meta = (DisplayName = "KillAllAgents"))
	void KillAllAgents();

	UFUNCTION(BlueprintCallable, Category = "BattleFrame | AgentSpawner", meta = (DisplayName = "KillAgentsBySubtype"))
	void KillAgentsBySubtype(int32 Index);

};