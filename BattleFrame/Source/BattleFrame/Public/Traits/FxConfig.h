#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "SubjectHandle.h"
#include "Traits/SubType.h"
#include "FxConfig.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FFxConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	bool bEnable = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "合批特效的子类型", DisplayName = "SubType_Batched"))
	EESubType SubType = EESubType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "非合批特效Niagara资产", DisplayName = "NiagaraAsset_UnBatched"))
	UNiagaraSystem* NiagaraAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "非合批特效Cascade资产", DisplayName = "CascadeAsset_UnBatched"))
	UParticleSystem* CascadeAsset = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	FTransform Transform = FTransform::Identity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	int32 Quantity = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = ""))
	float Delay = 0;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = ""))
	//ESpawnOrigin SpawnOrigin = ESpawnOrigin::AtSelf;

	//-----------------------------------------------

	float TimeLeft = 0;

	FSubjectHandle Owner = FSubjectHandle();

	FTransform SubjectTrans = FTransform::Identity;

};