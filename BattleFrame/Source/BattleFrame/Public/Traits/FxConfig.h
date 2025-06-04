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
struct BATTLEFRAME_API FFxConfig
{
	GENERATED_BODY()

public:

	// 默认构造函数
	FFxConfig() = default;

	// 从FActorSpawnConfig_Attack构造的构造函数
	FFxConfig(const FFxConfig_Attack& AttackConfig)
	{
		bEnable = AttackConfig.bEnable;
		SubType = AttackConfig.SubType;
		NiagaraAsset = AttackConfig.NiagaraAsset;
		CascadeAsset = AttackConfig.CascadeAsset;
		Transform = AttackConfig.Transform;
		Quantity = AttackConfig.Quantity;
		Delay = AttackConfig.Delay;
		LifeSpan = AttackConfig.LifeSpan;
		bAttached = AttackConfig.bAttached;
		bDespawnWhenNoParent = AttackConfig.bDespawnWhenNoParent;
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

	FSubjectHandle OwnerSubject = FSubjectHandle();
	FSubjectHandle AttachToSubject = FSubjectHandle();

	FTransform SpawnTransform;
	FTransform InitialRelativeTransform;

	TArray<UNiagaraComponent*> SpawnedNiagaraSystems;
	TArray<UParticleSystemComponent*> SpawnedCascadeSystems;

	bool bSpawned = false;
};