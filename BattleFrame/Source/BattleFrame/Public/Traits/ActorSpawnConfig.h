#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "BattleFrameEnums.h"
#include "ActorSpawnConfig.generated.h" 


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FActorSpawnConfig
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

};


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
struct BATTLEFRAME_API FActorSpawnConfig_Final
{
    GENERATED_BODY()

public:

    FActorSpawnConfig_Final() = default;

    FActorSpawnConfig_Final(const FActorSpawnConfig& Config)
    {
        bEnable = Config.bEnable;
        ActorClass = Config.ActorClass;
        Transform = Config.Transform;
        Quantity = Config.Quantity;
        Delay = Config.Delay;
        LifeSpan = Config.LifeSpan;
        bAttached = Config.bAttached;
        bDespawnWhenNoParent = Config.bDespawnWhenNoParent;
    }

    FActorSpawnConfig_Final(const FActorSpawnConfig_Attack& Config)
    {
        bEnable = Config.bEnable;
        ActorClass = Config.ActorClass;
        Transform = Config.Transform;
        Quantity = Config.Quantity;
        Delay = Config.Delay;
        LifeSpan = Config.LifeSpan;
        bAttached = Config.bAttached;
        bDespawnWhenNoParent = Config.bDespawnWhenNoParent;
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

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "生成者"))
    FSubjectHandle OwnerSubject = FSubjectHandle();

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "附着到"))
    FSubjectHandle AttachToSubject = FSubjectHandle();

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
    FTransform SpawnTransform;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
    FTransform InitialRelativeTransform;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
    TArray<AActor*> SpawnedActors;

    bool bInitialized = false;
    bool bSpawned = false;
};
