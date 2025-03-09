#pragma once
 
#include "CoreMinimal.h"
#include "Shooting.generated.h"


/**
 * The shooting ability.
 */
USTRUCT(BlueprintType)
struct BATTLEFRAME_API FShooting
{
	GENERATED_BODY()
 
  public:

	/**
	 * The shot's timeout to the next shot possibility.
	 */
	float Timeout = 0.0f;

	float retargettingTimeout = 0.0f;

	float sleepRetargettingTimeout = 0.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FSubjectHandle AimTarget = FSubjectHandle();

	FSubjectHandle NearestAgent = FSubjectHandle();

	bool bHasTarget = false;

	/* 弹匣子弹剩余数量*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 MagazineSkill1 = 0;

	/* 弹匣子弹剩余数量*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 MagazineSkill2 = 0;

	/* 攻击开关*/
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool bCanShoot = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float skill1RadiusBoss = 500.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Skill1RadiusNotBoss = 500.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ellipseMult = 1.5f;

	int32 previousShootMode = 0;

	bool shootFinished = false;

};