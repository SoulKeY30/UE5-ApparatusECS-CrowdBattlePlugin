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
#include "BattleFrameBattleControl.h"
#include "AgentSpawner.generated.h"


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSpawnerMult
{
	GENERATED_BODY()

public:

	// 成员变量默认值
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "血量乘数"))
	float HealthMult = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "速度乘数"))
	float MoveSpeedMult = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "伤害乘数"))
	float DamageMult = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "尺寸乘数"))
	float ScaleMult = 1.f;

	// 默认构造函数（必须）
	FSpawnerMult() = default;
};

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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Spawning)
	TArray<TSoftObjectPtr<UAgentConfigDataAsset>> AgentConfigAssets;

	UWorld* CurrentWorld = nullptr;
	AMechanism* Mechanism = nullptr;
	ABattleFrameBattleControl* BattleControl = nullptr;

	EFlagmarkBit RegisterMultipleFlag = EFlagmarkBit::M;

	UFUNCTION(BlueprintCallable, Category = "Spawning")
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

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void ActivateAgent(FSubjectHandle Agent);

	UFUNCTION(BlueprintCallable, Category = "SpawningTD")
	void KillAllAgents();

	UFUNCTION(BlueprintCallable, Category = "SpawningTD")
	void KillAgentsByIndex(int32 Index);

};