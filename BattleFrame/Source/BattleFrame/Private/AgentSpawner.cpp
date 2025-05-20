/**
* BattleFrame
* Created: 2025
* Author: Leroy Works, All Rights Reserved.
**/

#include "AgentSpawner.h"
#include "Traits/Directed.h"
#include "Traits/Damage.h"
#include "Traits/Collider.h"
#include "Traits/Located.h"
#include "Traits/SubType.h"
#include "Traits/Agent.h"
#include "Traits/Health.h"
#include "Traits/Moving.h"
#include "Traits/Appearing.h"
#include "Traits/Move.h"
#include "Traits/Dying.h"
#include "Traits/TextPopUp.h"
#include "Traits/Animation.h"
#include "Traits/Appear.h"
#include "Traits/Tracing.h"
#include "Traits/Avoiding.h"
#include "Traits/RegisterMultiple.h"
#include "Traits/Team.h"
#include "AnimToTextureDataAsset.h"
#include "NiagaraSubjectRenderer.h"
#include "BattleFrameFunctionLibraryRT.h"
#include "Traits/Activated.h"
#include "SubjectHandle.h"


AAgentSpawner::AAgentSpawner() 
{
    PrimaryActorTick.bCanEverTick = true;
}

void AAgentSpawner::BeginPlay()
{
    Super::BeginPlay();

    CurrentWorld = GetWorld();

    if (CurrentWorld)
    {
        Mechanism = UMachine::ObtainMechanism(CurrentWorld);
        BattleControl = Cast<ABattleFrameBattleControl>(UGameplayStatics::GetActorOfClass(CurrentWorld, ABattleFrameBattleControl::StaticClass()));
    }
}

TArray<FSubjectHandle> AAgentSpawner::SpawnAgentsRectangular
(
    bool bAutoActivate,
    int32 ConfigIndex,
    int32 Quantity,
    int32 Team,
    FVector Origin,
    FVector2D Region,
    FVector2D LaunchForce,
    EInitialDirection InitialDirection,
    FVector2D CustomDirection,
    FSpawnerMult Multipliers
)
{
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("SpawnAgentsRectangular");

    TArray<FSubjectHandle> SpawnedAgents;

    if (!CurrentWorld)
    {
        CurrentWorld = GetWorld();

        if (!CurrentWorld)
        {
            return SpawnedAgents;
        }
    }

    if (!Mechanism)
    {
        Mechanism = UMachine::ObtainMechanism(CurrentWorld);

        if (!Mechanism)
        {
            return SpawnedAgents;
        }
    }

    if (!BattleControl)
    {
        BattleControl = Cast<ABattleFrameBattleControl>(UGameplayStatics::GetActorOfClass(CurrentWorld, ABattleFrameBattleControl::StaticClass()));

        if (!BattleControl)
        {
            return SpawnedAgents;
        }
    }

    if (!AgentConfigAssets.IsValidIndex(ConfigIndex))
    {
        return SpawnedAgents;
    }

    UAgentConfigDataAsset* DataAsset = AgentConfigAssets[ConfigIndex].LoadSynchronous();

    if (!IsValid(DataAsset))
    {
        return SpawnedAgents;
    }

    FSubjectRecord AgentConfig;

    AgentConfig.SetTrait(DataAsset->Agent);
    AgentConfig.SetTrait(DataAsset->SubType);
    AgentConfig.SetTrait(FTeam(Team));
    AgentConfig.SetTrait(DataAsset->Collider);
    AgentConfig.SetTrait(FLocated());
    AgentConfig.SetTrait(FDirected());
    AgentConfig.SetTrait(DataAsset->Scale);
    AgentConfig.SetTrait(DataAsset->Health);
    AgentConfig.SetTrait(DataAsset->Damage);
    AgentConfig.SetTrait(DataAsset->Debuff);
    AgentConfig.SetTrait(DataAsset->Defence);
    AgentConfig.SetTrait(DataAsset->Sleep);
    AgentConfig.SetTrait(DataAsset->Move);
    AgentConfig.SetTrait(FMoving());
    AgentConfig.SetTrait(DataAsset->Patrol);
    AgentConfig.SetTrait(DataAsset->Navigation);
    AgentConfig.SetTrait(DataAsset->Avoidance);
    AgentConfig.SetTrait(DataAsset->Appear);
    AgentConfig.SetTrait(DataAsset->Trace);
    AgentConfig.SetTrait(FTracing());
    AgentConfig.SetTrait(DataAsset->Attack);
    AgentConfig.SetTrait(DataAsset->Hit);
    AgentConfig.SetTrait(DataAsset->Death);
    AgentConfig.SetTrait(DataAsset->Animation);
    AgentConfig.SetTrait(DataAsset->HealthBar);
    AgentConfig.SetTrait(DataAsset->TextPop);
    AgentConfig.SetTrait(DataAsset->Curves);
    AgentConfig.SetTrait(DataAsset->CustomData);

    // Apply Multipliers
    auto& HealthTrait = AgentConfig.GetTraitRef<FHealth>();
    HealthTrait.Current *= Multipliers.HealthMult;
    HealthTrait.Maximum *= Multipliers.HealthMult;

    auto& TextPopUp = AgentConfig.GetTraitRef<FTextPopUp>();
    TextPopUp.WhiteTextBelowPercent *= Multipliers.HealthMult;
    TextPopUp.OrangeTextAbovePercent *= Multipliers.HealthMult;

    auto& DamageTrait = AgentConfig.GetTraitRef<FDamage>();
    DamageTrait.Damage *= Multipliers.DamageMult;

    auto& ScaledTrait = AgentConfig.GetTraitRef<FScaled>();
    ScaledTrait.Factors *= Multipliers.ScaleMult;
    ScaledTrait.renderFactors = ScaledTrait.Factors;

    auto& MoveTrait = AgentConfig.GetTraitRef<FMove>();
    MoveTrait.MoveSpeed *= Multipliers.SpeedMult;

    auto& ColliderTrait = AgentConfig.GetTraitRef<FCollider>();
    ColliderTrait.Radius *= Multipliers.ScaleMult;

    while (SpawnedAgents.Num() < Quantity)// the following traits varies from agent to agent
    {
        FSubjectRecord Config = AgentConfig;

        auto& Located = Config.GetTraitRef<FLocated>();
        auto& Directed = Config.GetTraitRef<FDirected>();
        auto& Collider = Config.GetTraitRef<FCollider>();
        auto& Move = Config.GetTraitRef<FMove>();
        auto& Moving = Config.GetTraitRef<FMoving>();
        auto& Patrol = Config.GetTraitRef<FPatrol>();

        float RandomX = FMath::RandRange(-Region.X / 2, Region.X / 2);
        float RandomY = FMath::RandRange(-Region.Y / 2, Region.Y / 2);

        FVector SpawnPoint2D = Origin + FVector(RandomX, RandomY, 0);
        FVector SpawnPoint3D;

        if (Move.bCanFly)
        {
            Moving.FlyingHeight = FMath::RandRange(Move.FlyHeightRange.X, Move.FlyHeightRange.Y);
            SpawnPoint3D = FVector(SpawnPoint2D.X, SpawnPoint2D.Y, Moving.FlyingHeight + GetActorLocation().Z);
        }
        else
        {
            SpawnPoint3D = FVector(SpawnPoint2D.X, SpawnPoint2D.Y, Collider.Radius + GetActorLocation().Z);
        }

        Patrol.Origin = SpawnPoint3D;
        Move.Goal = SpawnPoint3D;

        Located.Location = SpawnPoint3D;
        Located.PreLocation = SpawnPoint3D;
        Located.InitialLocation = SpawnPoint3D;

        switch (InitialDirection)
        {
            case EInitialDirection::FacePlayer:
            {
                APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

                if (IsValid(PlayerPawn))
                {
                    FVector Delta = PlayerPawn->GetActorLocation() - SpawnPoint3D;
                    Directed.Direction = Delta.GetSafeNormal2D();
                }
                else
                {
                    Directed.Direction = GetActorForwardVector().GetSafeNormal2D();
                }
                break;
            }

            case EInitialDirection::FaceForward:
            {
                Directed.Direction = GetActorForwardVector().GetSafeNormal2D();
                break;
            }

            case EInitialDirection::CustomDirection:
            {
                Directed.Direction = FVector(CustomDirection,0).GetSafeNormal2D();
                break;
            }
        }

        if (LaunchForce.Size() > 0)
        {
            Moving.LaunchForce = Directed.Direction * LaunchForce.X + Directed.Direction.UpVector * LaunchForce.Y;
            Moving.bLaunching = true;
        }

        // Spawn using the modified record
        const auto Agent = Mechanism->SpawnSubject(Config);

        if (bAutoActivate)
        {
            ActivateAgent(Agent);
        }

        SpawnedAgents.Add(Agent);
    }

    return SpawnedAgents;
}

void AAgentSpawner::ActivateAgent( FSubjectHandle Agent )// strange apparatus bug : don't use get ref or the value may expire later when use
{
    auto Located = Agent.GetTrait<FLocated>();
    auto Collider = Agent.GetTrait<FCollider>();
    auto Appear = Agent.GetTrait<FAppear>();
    auto Sleep = Agent.GetTrait<FSleep>();
    auto Patrol = Agent.GetTrait<FPatrol>();
    auto Animation = Agent.GetTrait<FAnimation>();
    auto Team = Agent.GetTrait<FTeam>();
    auto SubType = Agent.GetTrait<FSubType>();
    auto Avoidance = Agent.GetTrait<FAvoidance>();

    Animation.AnimToTextureData = Animation.AnimToTextureDataAsset.LoadSynchronous(); // DataAsset Solid Pointer

    if (IsValid(Animation.AnimToTextureData))
    {
        for (FAnimToTextureAnimInfo CurrentAnim : Animation.AnimToTextureData->Animations)
        {
            Animation.AnimLengthArray.Add((CurrentAnim.EndFrame - CurrentAnim.StartFrame) / Animation.AnimToTextureData->SampleRate);
        }
    }

    if (Appear.bEnable)
    {
        Animation.Dissolve = 1;
        Agent.SetTrait(FAppearing());
    }

    Agent.SetTrait(Animation);

    if (Sleep.bEnable)
    {
        Agent.SetTrait(FSleeping());
    }

    if (Patrol.bEnable)
    {
        Agent.SetTrait(FPatrolling());
    }

    if (Collider.bHightQuality)
    {
        Agent.SetTrait(FRegisterMultiple());
    }

    Agent.SetTrait(FAvoiding{ Located.Location, Collider.Radius, Agent, Agent.CalcHash() });

    UBattleFrameFunctionLibraryRT::SetSubjectSubTypeTraitByIndex(SubType.Index, Agent);
    UBattleFrameFunctionLibraryRT::SetSubjectTeamTraitByIndex(FMath::Clamp(Team.index, 0, 9), Agent);
    UBattleFrameFunctionLibraryRT::SetSubjectAvoGroupTraitByIndex(FMath::Clamp(Avoidance.Group, 0, 9), Agent);

    // 如果场上没有，生成该怪物的渲染器
    if (CurrentWorld && BattleControl && !BattleControl->ExistingRenderers.Contains(SubType.Index))
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.bNoFail = true;

        // 直接生成目标类型
        ANiagaraSubjectRenderer* RendererActor = CurrentWorld->SpawnActor<ANiagaraSubjectRenderer>(Animation.RendererClass, FTransform::Identity, SpawnParams);

        // 设置SubType参数
        if (RendererActor)
        {
            BattleControl->ExistingRenderers.Add(SubType.Index);
            RendererActor->SubType.Index = SubType.Index;
            RendererActor->SetActorTickEnabled(true);
        }
    }

    Agent.SetTrait(FActivated());
}

void AAgentSpawner::KillAllAgents()
{
    CurrentWorld = GetWorld();

    if (CurrentWorld)
    {
        Mechanism = UMachine::ObtainMechanism(CurrentWorld);

        if (Mechanism)
        {
            FFilter Filter = FFilter::Make<FAgent>();
            Filter.Exclude<FDying>();

            Mechanism->Operate<FUnsafeChain>(Filter,
                [&](FUnsafeSubjectHandle Subject,
                    FAgent& Agent)
                {
                    Subject.SetTrait(FDying());
                });
        }
    }
}

void AAgentSpawner::KillAgentsByIndex(int32 Index)
{
    CurrentWorld = GetWorld();

    if (CurrentWorld)
    {
        Mechanism = UMachine::ObtainMechanism(CurrentWorld);

        if (Mechanism)
        {
            FFilter Filter = FFilter::Make<FAgent>().Exclude<FDying>();
            UBattleFrameFunctionLibraryRT::IncludeSubTypeTraitByIndex(Index, Filter);

            Mechanism->Operate<FUnsafeChain>(Filter,
                [&](FUnsafeSubjectHandle Subject)
                {
                    Subject.SetTrait(FDying());
                });
        }
    }
}
