/*
 * APPARATIST
 * Created: 2023-02-02 16:26:26
 * Author: Vladislav Dmitrievich Turbanov (vladislav@turbanov.ru)
 * Community forums: https://talk.turbanov.ru
 * Copyright 2019 - 2023, SP Vladislav Dmitrievich Turbanov
 * Made in Russia, Moscow City, Chekhov City
 */

 /*
  * BattleFrame
  * Refactor: 2025
  * Author: Leroy Works
  */

#include "NeighborGridActor.h"

ANeighborGridActor::ANeighborGridActor()
{
	PrimaryActorTick.bCanEverTick = false;
	const auto SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	NeighborGridComponent = CreateDefaultSubobject<UNeighborGridComponent>("NeighborGridComponent");
	SceneComponent->Mobility = EComponentMobility::Static;
	RootComponent = SceneComponent;
}

void ANeighborGridActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (NeighborGridComponent)
    {
        NeighborGridComponent->InitializeComponent();
    }
}
