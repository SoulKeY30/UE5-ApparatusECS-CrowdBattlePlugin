// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubjectiveActorComponent.h"
#include "BattleFrameSubjectiveActor.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEFRAME_API ABattleFrameSubjectiveActor : public AActor
{
	GENERATED_BODY()

public:

	ABattleFrameSubjectiveActor();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component)
	USubjectiveActorComponent* Subjective = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
