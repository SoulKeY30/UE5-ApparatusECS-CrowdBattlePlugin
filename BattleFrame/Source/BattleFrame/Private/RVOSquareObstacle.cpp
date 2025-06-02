/*
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
    BoxComponent->SetWorldRotation(FRotator(0, BoxComponent->GetComponentRotation().Yaw, 0));
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

    // 预计算所有点的2D位置
    RVO::Vector2 Point1(Point1Location.X, Point1Location.Y);
    RVO::Vector2 Point2(Point2Location.X, Point2Location.Y);
    RVO::Vector2 Point3(Point3Location.X, Point3Location.Y);
    RVO::Vector2 Point4(Point4Location.X, Point4Location.Y);

    FBoxObstacle BoxObstacle1
    {
        FSubjectHandle(),  // nextObstacle_ (将在后面设置)
        FSubjectHandle(),  // prevObstacle_ (将在后面设置)
        Point1,           // point_
        Point4,           // prePoint_
        Point2,           // nextPoint_
        Point3,           // nextNextPoint_
        RVO::Vector2((Point2Location - Point1Location).GetSafeNormal2D().X, (Point2Location - Point1Location).GetSafeNormal2D().Y), // unitDir_
        Point1Location.Z, // pointZ_
        HeightValue,      // height_
        true,            // isConvex_
        !bIsDynamicObstacle, // bStatic
        false,           // bRegistered
        bExcludeFromVisibilityCheck // bExcluded
    };

    FBoxObstacle BoxObstacle2
    {
        FSubjectHandle(),  // nextObstacle_
        FSubjectHandle(),  // prevObstacle_
        Point2,           // point_
        Point1,           // prePoint_
        Point3,           // nextPoint_
        Point4,           // nextNextPoint_
        RVO::Vector2((Point3Location - Point2Location).GetSafeNormal2D().X, (Point3Location - Point2Location).GetSafeNormal2D().Y), // unitDir_
        Point2Location.Z, // pointZ_
        HeightValue,      // height_
        true,            // isConvex_
        !bIsDynamicObstacle, // bStatic
        false,           // bRegistered
        bExcludeFromVisibilityCheck // bExcluded
    };

    FBoxObstacle BoxObstacle3
    {
        FSubjectHandle(),  // nextObstacle_
        FSubjectHandle(),  // prevObstacle_
        Point3,           // point_
        Point2,           // prePoint_
        Point4,           // nextPoint_
        Point1,           // nextNextPoint_
        RVO::Vector2((Point4Location - Point3Location).GetSafeNormal2D().X, (Point4Location - Point3Location).GetSafeNormal2D().Y), // unitDir_
        Point3Location.Z, // pointZ_
        HeightValue,      // height_
        true,            // isConvex_
        !bIsDynamicObstacle, // bStatic
        false,           // bRegistered
        bExcludeFromVisibilityCheck // bExcluded
    };

    FBoxObstacle BoxObstacle4
    {
        FSubjectHandle(),  // nextObstacle_
        FSubjectHandle(),  // prevObstacle_
        Point4,           // point_
        Point3,           // prePoint_
        Point1,           // nextPoint_
        Point2,           // nextNextPoint_
        RVO::Vector2((Point1Location - Point4Location).GetSafeNormal2D().X, (Point1Location - Point4Location).GetSafeNormal2D().Y), // unitDir_
        Point4Location.Z, // pointZ_
        HeightValue,      // height_
        true,            // isConvex_
        !bIsDynamicObstacle, // bStatic
        false,           // bRegistered
        bExcludeFromVisibilityCheck // bExcluded
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

    // 设置障碍物之间的连接关系
    Obstacle1.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>()->prevObstacle_ = Obstacle4;
    Obstacle1.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>()->nextObstacle_ = Obstacle2;

    Obstacle2.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>()->prevObstacle_ = Obstacle1;
    Obstacle2.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>()->nextObstacle_ = Obstacle3;

    Obstacle3.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>()->prevObstacle_ = Obstacle2;
    Obstacle3.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>()->nextObstacle_ = Obstacle4;

    Obstacle4.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>()->prevObstacle_ = Obstacle3;
    Obstacle4.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>()->nextObstacle_ = Obstacle1;

    // 设置网格数据
    Obstacle1.SetTrait(FGridData{ Obstacle1.CalcHash(), FVector3f(Point1Location), 0, Obstacle1 });
    Obstacle2.SetTrait(FGridData{ Obstacle2.CalcHash(), FVector3f(Point2Location), 0, Obstacle2 });
    Obstacle3.SetTrait(FGridData{ Obstacle3.CalcHash(), FVector3f(Point3Location), 0, Obstacle3 });
    Obstacle4.SetTrait(FGridData{ Obstacle4.CalcHash(), FVector3f(Point4Location), 0, Obstacle4 });
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

        // 更新位置数据
        Obstacle1.GetTraitPtr<FLocated, EParadigm::Unsafe>()->Location = Point1Location;
        Obstacle2.GetTraitPtr<FLocated, EParadigm::Unsafe>()->Location = Point2Location;
        Obstacle3.GetTraitPtr<FLocated, EParadigm::Unsafe>()->Location = Point3Location;
        Obstacle4.GetTraitPtr<FLocated, EParadigm::Unsafe>()->Location = Point4Location;

        // 预计算所有点的2D位置
        RVO::Vector2 Point1(Point1Location.X, Point1Location.Y);
        RVO::Vector2 Point2(Point2Location.X, Point2Location.Y);
        RVO::Vector2 Point3(Point3Location.X, Point3Location.Y);
        RVO::Vector2 Point4(Point4Location.X, Point4Location.Y);

        auto UpdateObstacle = [&](FSubjectHandle Obstacle, const FVector& Location, const RVO::Vector2& Point,
            const RVO::Vector2& PrePoint, const RVO::Vector2& NextPoint,
            const RVO::Vector2& NextNextPoint, const FVector& NextLocation)
            {
                FBoxObstacle* ObstacleData = Obstacle.GetTraitPtr<FBoxObstacle, EParadigm::Unsafe>();

                ObstacleData->point_ = Point;
                ObstacleData->prePoint_ = PrePoint;
                ObstacleData->nextPoint_ = NextPoint;
                ObstacleData->nextNextPoint_ = NextNextPoint;
                ObstacleData->unitDir_ = RVO::Vector2((NextLocation - Location).GetSafeNormal2D().X,
                    (NextLocation - Location).GetSafeNormal2D().Y);
                ObstacleData->pointZ_ = Location.Z;
                ObstacleData->height_ = CurrentHeight;
                ObstacleData->bExcluded = bExcludeFromVisibilityCheck;
            };

        UpdateObstacle(Obstacle1, Point1Location, Point1, Point4, Point2, Point3, Point2Location);
        UpdateObstacle(Obstacle2, Point2Location, Point2, Point1, Point3, Point4, Point3Location);
        UpdateObstacle(Obstacle3, Point3Location, Point3, Point2, Point4, Point1, Point4Location);
        UpdateObstacle(Obstacle4, Point4Location, Point4, Point3, Point1, Point2, Point1Location);
    }
}
