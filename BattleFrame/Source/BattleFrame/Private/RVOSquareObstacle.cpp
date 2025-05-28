﻿/*
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
*/

#include "RVOSquareObstacle.h"
#include "BattleFrameBattleControl.h"

// Sets default values 
ARVOSquareObstacle::ARVOSquareObstacle()
{
    PrimaryActorTick.bCanEverTick = false;
    
    BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
    RootComponent = BoxComponent;
}

void ARVOSquareObstacle::OnConstruction(const FTransform& Transform)
{
    //BoxComponent->SetWorldScale3D(FVector(1,1,1));
    BoxComponent->SetWorldRotation(FRotator(0,BoxComponent->GetComponentRotation().Yaw,0));
}

void ARVOSquareObstacle::BeginPlay()
{
    Super::BeginPlay();

    FTransform ComponentTransform = GetActorTransform();
    FVector BoxExtent = BoxComponent->GetUnscaledBoxExtent();
    const float HeightValue = BoxExtent.Z * 2 * ComponentTransform.GetScale3D().Z;

    FVector LocalPoint1 = FVector(-BoxExtent.X, -BoxExtent.Y, 0);
    FVector LocalPoint2 = FVector(BoxExtent.X, -BoxExtent.Y, 0);
    FVector LocalPoint3 = FVector(BoxExtent.X, BoxExtent.Y, 0);
    FVector LocalPoint4 = FVector(-BoxExtent.X, BoxExtent.Y, 0);

    FVector Point1Location = ComponentTransform.TransformPosition(LocalPoint1);
    FVector Point2Location = ComponentTransform.TransformPosition(LocalPoint2);
    FVector Point3Location = ComponentTransform.TransformPosition(LocalPoint3);
    FVector Point4Location = ComponentTransform.TransformPosition(LocalPoint4);

    if (bInsideOut)
    {
        Swap(Point2Location, Point4Location);
    }

    FLocated Located1{ Point1Location };
    FLocated Located2{ Point2Location };
    FLocated Located3{ Point3Location };
    FLocated Located4{ Point4Location };

    FBoxObstacle BoxObstacle1
    {
        true,
        FSubjectHandle(),
        RVO::Vector2(Point1Location.X, Point1Location.Y),
        FSubjectHandle(),
        RVO::Vector2((Point2Location - Point1Location).GetSafeNormal2D().X, (Point2Location - Point1Location).GetSafeNormal2D().Y),
        Point1Location,
        HeightValue,
        !bIsDynamicObstacle,
        false,
        bExcludeFromVisibilityCheck
    };

    FBoxObstacle BoxObstacle2
    {
        true,
        FSubjectHandle(),
        RVO::Vector2(Point2Location.X, Point2Location.Y),
        FSubjectHandle(),
        RVO::Vector2((Point3Location - Point2Location).GetSafeNormal2D().X, (Point3Location - Point2Location).GetSafeNormal2D().Y),
        Point2Location,
        HeightValue,
        !bIsDynamicObstacle,
        false,
        bExcludeFromVisibilityCheck
    };

    FBoxObstacle BoxObstacle3
    {
        true,
        FSubjectHandle(),
        RVO::Vector2(Point3Location.X, Point3Location.Y),
        FSubjectHandle(),
        RVO::Vector2((Point4Location - Point3Location).GetSafeNormal2D().X, (Point4Location - Point3Location).GetSafeNormal2D().Y),
        Point3Location,
        HeightValue,
        !bIsDynamicObstacle,
        false,
        bExcludeFromVisibilityCheck
    };

    FBoxObstacle BoxObstacle4
    {
        true,
        FSubjectHandle(),
        RVO::Vector2(Point4Location.X, Point4Location.Y),
        FSubjectHandle(),
        RVO::Vector2((Point1Location - Point4Location).GetSafeNormal2D().X, (Point1Location - Point4Location).GetSafeNormal2D().Y),
        Point4Location,
        HeightValue,
        !bIsDynamicObstacle,
        false,
        bExcludeFromVisibilityCheck
    };

    FSubjectRecord Record1;
    FSubjectRecord Record2;
    FSubjectRecord Record3;
    FSubjectRecord Record4;

    Record1.SetTrait(Located1);
    Record2.SetTrait(Located2);
    Record3.SetTrait(Located3);
    Record4.SetTrait(Located4);

    Record1.SetTrait(BoxObstacle1);
    Record2.SetTrait(BoxObstacle2);
    Record3.SetTrait(BoxObstacle3);
    Record4.SetTrait(BoxObstacle4);

    // Spawn Subjects
    AMechanism* Mechanism = UMachine::ObtainMechanism(GetWorld());

    Obstacle1 = Mechanism->SpawnSubject(Record1);
    Obstacle2 = Mechanism->SpawnSubject(Record2);
    Obstacle3 = Mechanism->SpawnSubject(Record3);
    Obstacle4 = Mechanism->SpawnSubject(Record4);

    Obstacle1.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>()->prevObstacle_ = Obstacle4;
    Obstacle1.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>()->nextObstacle_ = Obstacle2;

    Obstacle2.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>()->prevObstacle_ = Obstacle1;
    Obstacle2.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>()->nextObstacle_ = Obstacle3;

    Obstacle3.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>()->prevObstacle_ = Obstacle2;
    Obstacle3.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>()->nextObstacle_ = Obstacle4;

    Obstacle4.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>()->prevObstacle_ = Obstacle3;
    Obstacle4.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>()->nextObstacle_ = Obstacle1;

    Obstacle1.SetTrait(FGridData{ FVector3f(Point1Location), 0, Obstacle1, Obstacle1.CalcHash() });
    Obstacle2.SetTrait(FGridData{ FVector3f(Point2Location), 0, Obstacle2, Obstacle2.CalcHash() });
    Obstacle3.SetTrait(FGridData{ FVector3f(Point3Location), 0, Obstacle3, Obstacle3.CalcHash() });
    Obstacle4.SetTrait(FGridData{ FVector3f(Point4Location), 0, Obstacle4, Obstacle4.CalcHash() });
}

void ARVOSquareObstacle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (Obstacle1.IsValid())
    {
        Obstacle1->DespawnDeferred();
    }

    if (Obstacle2.IsValid())
    {
        Obstacle2->DespawnDeferred();
    }

    if (Obstacle3.IsValid())
    {
        Obstacle3->DespawnDeferred();
    }

    if (Obstacle4.IsValid())
    {
        Obstacle4->DespawnDeferred();
    }
}

void ARVOSquareObstacle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDynamicObstacle)
    {
        FTransform ComponentTransform = BoxComponent->GetComponentTransform();
        FVector BoxExtent = BoxComponent->GetUnscaledBoxExtent();
        const float CurrentHeight = BoxExtent.Z * 2 * ComponentTransform.GetScale3D().Z;

        FVector LocalPoint1 = FVector(-BoxExtent.X, -BoxExtent.Y, 0);
        FVector LocalPoint2 = FVector(BoxExtent.X, -BoxExtent.Y, 0);
        FVector LocalPoint3 = FVector(BoxExtent.X, BoxExtent.Y, 0);
        FVector LocalPoint4 = FVector(-BoxExtent.X, BoxExtent.Y, 0);

        FVector Point1Location = ComponentTransform.TransformPosition(LocalPoint1);
        FVector Point2Location = ComponentTransform.TransformPosition(LocalPoint2);
        FVector Point3Location = ComponentTransform.TransformPosition(LocalPoint3);
        FVector Point4Location = ComponentTransform.TransformPosition(LocalPoint4);

        if (bInsideOut)
        {
            Swap(Point2Location, Point4Location);
        }

        Obstacle1.GetTraitPtr<FLocated, EParadigm::Unsafe>()->Location = Point1Location;
        Obstacle2.GetTraitPtr<FLocated, EParadigm::Unsafe>()->Location = Point2Location;
        Obstacle3.GetTraitPtr<FLocated, EParadigm::Unsafe>()->Location = Point3Location;
        Obstacle4.GetTraitPtr<FLocated, EParadigm::Unsafe>()->Location = Point4Location;

        auto UpdateObstacle = [&](FSubjectHandle Obstacle, const FVector& Location, const FVector& NextLocation) 
        {
            FBoxObstacle* ObstacleData = Obstacle.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();

            ObstacleData->point3d_ = Location;
            ObstacleData->point_ = RVO::Vector2(Location.X, Location.Y);
            ObstacleData->unitDir_ = RVO::Vector2((NextLocation - Location).GetSafeNormal2D().X, (NextLocation - Location).GetSafeNormal2D().Y);
            ObstacleData->height_ = CurrentHeight;
            ObstacleData->bExcluded = bExcludeFromVisibilityCheck;
        };

        UpdateObstacle(Obstacle1, Point1Location, Point2Location);
        UpdateObstacle(Obstacle2, Point2Location, Point3Location);
        UpdateObstacle(Obstacle3, Point3Location, Point4Location);
        UpdateObstacle(Obstacle4, Point4Location, Point1Location);
    }
}
