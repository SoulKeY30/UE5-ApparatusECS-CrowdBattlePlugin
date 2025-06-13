#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "SubjectHandle.h"
#include "Traits/SubType.h"
#include "BattleFrameEnums.h"
#include <NiagaraComponent.h>
#include <Particles/ParticleSystemComponent.h>
#include "FxConfig.generated.h" 


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FFxConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "启用"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "合批特效的子类型,不支持Attach", DisplayName = "SubType_Batched"))
	EESubType SubType = EESubType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "非合批特效Niagara资产", DisplayName = "NiagaraAsset_UnBatched"))
	UNiagaraSystem* NiagaraAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "非合批特效Cascade资产", DisplayName = "CascadeAsset_UnBatched"))
	UParticleSystem* CascadeAsset = nullptr;

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
struct BATTLEFRAME_API FFxConfig_Attack
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "启用"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "合批特效的子类型,不支持Attach", DisplayName = "SubType_Batched"))
	EESubType SubType = EESubType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "非合批特效Niagara资产", DisplayName = "NiagaraAsset_UnBatched"))
	UNiagaraSystem* NiagaraAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "非合批特效Cascade资产", DisplayName = "CascadeAsset_UnBatched"))
	UParticleSystem* CascadeAsset = nullptr;

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
struct BATTLEFRAME_API FFxConfig_Final
{
	GENERATED_BODY()

public:

	FFxConfig_Final() = default;

	FFxConfig_Final(const FFxConfig& Config)
	{
		bEnable = Config.bEnable;
		SubType = Config.SubType;
		NiagaraAsset = Config.NiagaraAsset;
		CascadeAsset = Config.CascadeAsset;
		Transform = Config.Transform;
		Quantity = Config.Quantity;
		Delay = Config.Delay;
		LifeSpan = Config.LifeSpan;
		bAttached = Config.bAttached;
		bDespawnWhenNoParent = Config.bDespawnWhenNoParent;
	}

	FFxConfig_Final(const FFxConfig_Attack& Config)
	{
		bEnable = Config.bEnable;
		SubType = Config.SubType;
		NiagaraAsset = Config.NiagaraAsset;
		CascadeAsset = Config.CascadeAsset;
		Transform = Config.Transform;
		Quantity = Config.Quantity;
		Delay = Config.Delay;
		LifeSpan = Config.LifeSpan;
		bAttached = Config.bAttached;
		bDespawnWhenNoParent = Config.bDespawnWhenNoParent;
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "启用"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "合批特效的子类型", DisplayName = "SubType_Batched"))
	EESubType SubType = EESubType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "非合批特效Niagara资产", DisplayName = "NiagaraAsset_UnBatched"))
	UNiagaraSystem* NiagaraAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "非合批特效Cascade资产", DisplayName = "CascadeAsset_UnBatched"))
	UParticleSystem* CascadeAsset = nullptr;

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
	TArray<UNiagaraComponent*> SpawnedNiagaraSystems;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	TArray<UParticleSystemComponent*> SpawnedCascadeSystems;

	bool bInitialized = false;
	bool bSpawned = false;

	float LaunchSpeed = 0;
};