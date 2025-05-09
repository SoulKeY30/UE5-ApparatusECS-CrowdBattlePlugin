/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"

#include "Traits/Avoidance.h"
#include "Traits/Avoiding.h"
#include "Traits/Located.h"
#include "Traits/Collider.h"
#include "Traits/SphereObstacle.h"
#include "Traits/RegisterMultiple.h"

#include "RVOSphereObstacle.generated.h"

UCLASS(Blueprintable)
class BATTLEFRAME_API ARVOSphereObstacle : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARVOSphereObstacle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVOSphereObstacle")
	bool bIsDynamicObstacle = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVOSphereObstacle")
	bool bOverrideSpeedLimit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVOSphereObstacle")
	float NewSpeedLimit = 2000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVOSphereObstacle")
	bool bExcludeFromVisibilityCheck = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RVOSphereObstacle")
	USphereComponent* SphereComponent;

	FSubjectHandle SubjectHandle;

};
