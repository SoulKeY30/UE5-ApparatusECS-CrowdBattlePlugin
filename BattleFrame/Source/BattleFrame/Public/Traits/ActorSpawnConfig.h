#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "BattleFrameEnums.h"
#include "ActorSpawnConfig.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FActorSpawnConfig_Attack
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "启用"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "Actor类"))
	TSubclassOf<AActor> ActorClass = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "偏移量"))
	FTransform Transform = FTransform::Identity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "数量"))
	int32 Quantity = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "延时生成"))
	float Delay = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "生成后存活时长,负值为无限长"))
    float LifeSpan = 5;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "附着"))
    bool bAttached = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "附着点失效后自毁"))
    bool bDespawnWhenNoParent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "原点"))
	ESpawnOrigin SpawnOrigin = ESpawnOrigin::AtSelf;

};


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FActorSpawnConfig
{
    GENERATED_BODY()

public:
    // 默认构造函数
    FActorSpawnConfig() = default;

    // 从FActorSpawnConfig_Attack构造的构造函数
    FActorSpawnConfig(const FActorSpawnConfig_Attack& AttackConfig)
    {
        bEnable = AttackConfig.bEnable;
        ActorClass = AttackConfig.ActorClass;
        Transform = AttackConfig.Transform;
        Quantity = AttackConfig.Quantity;
        Delay = AttackConfig.Delay;
        LifeSpan = AttackConfig.LifeSpan;
        bAttached = AttackConfig.bAttached;
        bDespawnWhenNoParent = AttackConfig.bDespawnWhenNoParent;
    }

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "启用"))
    bool bEnable = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "Actor类"))
    TSubclassOf<AActor> ActorClass = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "偏移量"))
    FTransform Transform = FTransform::Identity;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "数量"))
    int32 Quantity = 1;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "延时生成"))
    float Delay = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "生成后存活时长,负值为无限长"))
    float LifeSpan = 5;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "附着"))
    bool bAttached = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "附着点失效后自毁"))
    bool bDespawnWhenNoParent = true;


    //-----------------------------------------------

    FSubjectHandle OwnerSubject = FSubjectHandle();
    FSubjectHandle AttachToSubject = FSubjectHandle();

    FTransform SpawnTransform;
    FTransform InitialRelativeTransform;

    TArray<AActor*> SpawnedActors;

    bool bInitialized = false;
    bool bSpawned = false;
};
