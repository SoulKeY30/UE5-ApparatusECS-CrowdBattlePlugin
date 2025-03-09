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
	float SpeedMult = 1.f;

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
		DisplayName = "TowardsPlayer0",
		Tooltip = "生成时面朝玩家0（第一个玩家控制器）的方向\n适用于需要立即面对玩家的场景"
	),
	FaceForward UMETA(
		DisplayName = "SpawnerForwardVector",
		Tooltip = "使用生成器Actor的前向向量作为朝向\n适用于需要保持与生成器方向一致的布局"
	),
	FaceLocation UMETA(
		DisplayName = "TowardsCustomLocation",
		Tooltip = "面向自定义世界坐标位置\n需在FaceCustomLocation参数中指定目标位置"
	)
};


UCLASS()
class BATTLEFRAME_API AAgentSpawner : public AActor
{
	GENERATED_BODY()

private:

	static AAgentSpawner* Instance;


protected:

	void BeginPlay() override
	{
		Super::BeginPlay();
		Instance = this;
	}

	void EndPlay(const EEndPlayReason::Type EndPlayReason) override
	{
		if (Instance == this)
		{
			Instance = nullptr;
		}
		Super::EndPlay(EndPlayReason);
	}

	void Tick(float DeltaTime) override;

public:

	AAgentSpawner();

	UPROPERTY(EditAnywhere, Category = Spawning)
	TArray<TSoftObjectPtr<UAgentConfigDataAsset>> AgentConfigAssets;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static AAgentSpawner* GetInstance() { return Instance; }

	static FVector2D VRand2D()
	{
		FVector2D Result;
		float Length;

		do
		{
			Result.X = FMath::FRand() * 2.0f - 1.0f;
			Result.Y = FMath::FRand() * 2.0f - 1.0f;
			Length = Result.SizeSquared();
		} while ((Length > 1.0f) || (Length < KINDA_SMALL_NUMBER));

		return Result * (1.0f / FMath::Sqrt(Length));
	}

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	TArray<FSubjectHandle> SpawnAgentsRectangular(
		int32 configIndex = 0,
		int32 quantity = 1,
		int32 team = 0,
		FVector origin = FVector::ZeroVector,
		FVector2D region = FVector2D(0.f, 0.f),
		float LaunchForce = 0.f,
		EInitialDirection initialDirection = EInitialDirection::FacePlayer,
		FVector FaceCustomLocation = FVector::ZeroVector,
		FSpawnerMult Multipliers = FSpawnerMult()
	);

	UFUNCTION(BlueprintCallable, Category = "SpawningTD")
	void KillAllAgents();

	UFUNCTION(BlueprintCallable, Category = "SpawningTD")
	void KillAgentsByIndex(int32 Index);

};