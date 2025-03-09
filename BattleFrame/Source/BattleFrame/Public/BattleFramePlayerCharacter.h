/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SubjectiveActorComponent.h"
#include "Kismet/GameplayStatics.h"



#include "BattleFramePlayerCharacter.generated.h"

UCLASS()
class BATTLEFRAME_API ABattleFramePlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABattleFramePlayerCharacter();
	/**
	 * The subjective of the player pawn.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component)
	USubjectiveActorComponent* Subjective = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
