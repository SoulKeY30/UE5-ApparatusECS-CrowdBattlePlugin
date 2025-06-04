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
#include "BattleFrameBattleControl.h"
#include "AgentSpawner.generated.h"


UENUM(BlueprintType)
enum class EInitialDirection : uint8
{
	FacePlayer UMETA(
		DisplayName = "FacingPlayer0",
		Tooltip = "生成时面朝玩家0的方向"
	),
	FaceForward UMETA(
		DisplayName = "SpawnerForwardVector",
		Tooltip = "使用生成器的前向向量作为朝向"
	),
	CustomDirection UMETA(
		DisplayName = "CustomDirection",
		Tooltip = "自定义朝向"
	)
};


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

	UFUNCTION(BlueprintCallable, Category = "BattleFrame | AgentSpawner")
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
		FVector2D CustomDirection = FVector2D(1,0),
		FSpawnerMult Multipliers = FSpawnerMult()
	);

	UFUNCTION(BlueprintCallable, Category = "BattleFrame | AgentSpawner")
	void ActivateAgent(FSubjectHandle Agent);

	UFUNCTION(BlueprintCallable, Category = "BattleFrame | AgentSpawner")
	void KillAllAgents();

	UFUNCTION(BlueprintCallable, Category = "BattleFrame | AgentSpawner")
	void KillAgentsByIndex(int32 Index);

};