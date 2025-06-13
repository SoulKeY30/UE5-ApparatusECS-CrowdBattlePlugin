#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundBase.h"
#include "SubjectHandle.h"
#include "BattleFrameEnums.h"
#include "Components/AudioComponent.h"
#include "SoundConfig.generated.h" 


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSoundConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "启用"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "音效"))
	TSoftObjectPtr<USoundBase> Sound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "偏移量"))
	FTransform Transform = FTransform::Identity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "延时生成"))
	float Delay = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "音量"))
	float Volume = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "生成后存活时长,负值为无限长"))
	float LifeSpan = 5;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "附着"))
	bool bAttached = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "附着点失效后自毁"))
	bool bDespawnWhenNoParent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "原点"))
	EPlaySoundOrigin SpawnOrigin = EPlaySoundOrigin::PlaySound2D;

};


USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSoundConfig_Attack
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "启用"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "音效"))
	TSoftObjectPtr<USoundBase> Sound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "偏移量"))
	FTransform Transform = FTransform::Identity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "延时生成"))
	float Delay = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "音量"))
	float Volume = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "生成后存活时长,负值为无限长"))
	float LifeSpan = 5;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "附着"))
	bool bAttached = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "附着点失效后自毁"))
	bool bDespawnWhenNoParent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "原点"))
	EPlaySoundOrigin_Attack SpawnOrigin = EPlaySoundOrigin_Attack::PlaySound2D;

};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSoundConfig_Final
{
	GENERATED_BODY()

public:

	// 默认构造函数
	FSoundConfig_Final() = default;

	// 从FSoundSpawnConfig_Common构造的构造函数
	FSoundConfig_Final(const FSoundConfig& Config)
	{
		bEnable = Config.bEnable;
		Sound = Config.Sound;
		Transform = Config.Transform;
		Delay = Config.Delay;
		Volume = Config.Volume;
		LifeSpan = Config.LifeSpan;
		bAttached = Config.bAttached;
		bDespawnWhenNoParent = Config.bDespawnWhenNoParent;
		SpawnOrigin = Config.SpawnOrigin;
	}

	// 从FSoundSpawnConfig_Attack构造的构造函数
	FSoundConfig_Final(const FSoundConfig_Attack& AttackConfig)
	{
		bEnable = AttackConfig.bEnable;
		Sound = AttackConfig.Sound;
		Transform = AttackConfig.Transform;
		Delay = AttackConfig.Delay;
		Volume = AttackConfig.Volume;
		LifeSpan = AttackConfig.LifeSpan;
		bAttached = AttackConfig.bAttached;
		bDespawnWhenNoParent = AttackConfig.bDespawnWhenNoParent;

		// 安全转换SpawnOrigin
		switch (AttackConfig.SpawnOrigin)
		{
			case EPlaySoundOrigin_Attack::PlaySound2D:
				SpawnOrigin = EPlaySoundOrigin::PlaySound2D;
				break;
			case EPlaySoundOrigin_Attack::PlaySound3D_AtSelf:
				SpawnOrigin = EPlaySoundOrigin::PlaySound3D;
				break;
			case EPlaySoundOrigin_Attack::PlaySound3D_AtTarget:
				SpawnOrigin = EPlaySoundOrigin::PlaySound3D;
				break;
		}
	}


	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "启用"))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "音效"))
	TSoftObjectPtr<USoundBase> Sound = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "偏移量"))
	FTransform Transform = FTransform::Identity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "延时生成"))
	float Delay = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "音量"))
	float Volume = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "生成后存活时长,负值为无限长"))
	float LifeSpan = 5;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "附着"))
	bool bAttached = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "附着点失效后自毁"))
	bool bDespawnWhenNoParent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "原点"))
	EPlaySoundOrigin SpawnOrigin = EPlaySoundOrigin::PlaySound2D;

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
	TArray<UAudioComponent*> SpawnedSounds;

	bool bInitialized = false;
	bool bSpawned = false;

};
