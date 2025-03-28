
/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "SubjectiveActorComponent.h"
#include "Traits/Avoidance.h"
#include "Traits/BoxObstacle.h"
#include "Traits/Located.h"
#include "RVOSquareObstacle.generated.h"
 
UCLASS(Blueprintable)
class BATTLEFRAME_API ARVOSquareObstacle : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ARVOSquareObstacle();

    virtual void OnConstruction(const FTransform& Transform) override;

      
protected:
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVOSquareObstacle")
    bool bInsideOut = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVOSquareObstacle")
    bool bIsDynamicObstacle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVOSquareObstacle")
    UBoxComponent* BoxComponent;

    UPROPERTY(BlueprintReadOnly, Category = "RVOSquareObstacle")
    FSubjectHandle Obstacle1;

    UPROPERTY(BlueprintReadOnly, Category = "RVOSquareObstacle")
    FSubjectHandle Obstacle2;

    UPROPERTY(BlueprintReadOnly, Category = "RVOSquareObstacle")
    FSubjectHandle Obstacle3;

    UPROPERTY(BlueprintReadOnly, Category = "RVOSquareObstacle")
    FSubjectHandle Obstacle4;
};