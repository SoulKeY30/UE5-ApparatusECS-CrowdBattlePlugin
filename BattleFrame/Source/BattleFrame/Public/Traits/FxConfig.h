#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"

#include "FxConfig.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FFxConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "合批特效的子类型", DisplayName = "SubType_Batched"))
	ESubType SubType = ESubType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "非合批特效Niagara资产", DisplayName = "NiagaraAsset_UnBatched"))
	UNiagaraSystem* NiagaraAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "非合批特效Cascade资产", DisplayName = "CascadeAsset_UnBatched"))
	UParticleSystem* CascadeAsset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "偏移相对位置与朝向，全局缩放"))
	FTransform Transform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Tooltip = "是否固定"))
	bool bAttached = false;

};
