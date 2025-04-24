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
#include "DmgResultInterface.h"
#include "Traits/DmgResult.h"
#include "BattleFrameCharacter.generated.h"

UCLASS()
class BATTLEFRAME_API ABattleFrameCharacter : public ACharacter, public IDmgResultInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABattleFrameCharacter();
	/**
	 * The subjective of the player pawn.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component)
	USubjectiveActorComponent* Subjective = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//virtual void OnConstruction(const FTransform& Transform) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void ReceiveDamage_Implementation(const FDmgResult& DmgResult) override;

};
