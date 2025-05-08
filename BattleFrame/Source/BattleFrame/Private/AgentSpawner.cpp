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


AAgentSpawner* AAgentSpawner::Instance = nullptr;

AAgentSpawner::AAgentSpawner() 
{
    PrimaryActorTick.bCanEverTick = true;
}

void AAgentSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
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
    FVector FaceCustomLocation,
    FSpawnerMult Multipliers
)
{
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("SpawnAgentsRectangular");

    TArray<FSubjectHandle> SpawnedAgents;

    const auto Mechanism = UMachine::ObtainMechanism(GetWorld());
    if (!Mechanism) return SpawnedAgents;

    ConfigIndex = FMath::Clamp(ConfigIndex, 0, AgentConfigAssets.Num() - 1);
    if (!AgentConfigAssets.IsValidIndex(ConfigIndex)) return SpawnedAgents;

    UAgentConfigDataAsset* DataAsset = AgentConfigAssets[ConfigIndex].LoadSynchronous();
    if (!IsValid(DataAsset)) return SpawnedAgents;

    // 如果场上没有，生成该怪物的渲染器
    ANiagaraSubjectRenderer* RendererActor = Cast<ANiagaraSubjectRenderer>(UGameplayStatics::GetActorOfClass(GetWorld(), DataAsset->Animation.RendererClass));

    if (!RendererActor && DataAsset->Animation.RendererClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.bNoFail = true;

        // 直接生成目标类型
        RendererActor = GetWorld()->SpawnActor<ANiagaraSubjectRenderer>(DataAsset->Animation.RendererClass, FTransform::Identity, SpawnParams);

        // 设置SubType参数
        if (RendererActor)
        {
            RendererActor->SubType.Index = DataAsset->SubType.Index;
            RendererActor->TickEnabled = true;
        }
    }

    FSubjectRecord AgentConfig;

    AgentConfig.SetTrait(DataAsset->Agent);
    AgentConfig.SetTrait(DataAsset->Collider);
    AgentConfig.SetTrait(DataAsset->Scale);
    AgentConfig.SetTrait(DataAsset->Health);
    AgentConfig.SetTrait(DataAsset->Damage);
    AgentConfig.SetTrait(DataAsset->Debuff);
    AgentConfig.SetTrait(DataAsset->Defence);
    AgentConfig.SetTrait(DataAsset->Sleep);
    AgentConfig.SetTrait(DataAsset->Move);
    AgentConfig.SetTrait(DataAsset->Patrol);
    AgentConfig.SetTrait(DataAsset->Navigation);
    AgentConfig.SetTrait(DataAsset->Avoidance);
    AgentConfig.SetTrait(DataAsset->Appear);
    AgentConfig.SetTrait(DataAsset->Trace);
    AgentConfig.SetTrait(DataAsset->Attack);
    AgentConfig.SetTrait(DataAsset->Hit);
    AgentConfig.SetTrait(DataAsset->Death);
    AgentConfig.SetTrait(DataAsset->Animation);
    AgentConfig.SetTrait(DataAsset->HealthBar);
    AgentConfig.SetTrait(DataAsset->TextPop);
    AgentConfig.SetTrait(DataAsset->Curves);
    AgentConfig.SetTrait(DataAsset->CustomData);

    AgentConfig.SetTrait(FLocated());
    AgentConfig.SetTrait(FDirected());
    AgentConfig.SetTrait(FTracing());
    AgentConfig.SetTrait(FMoving());
    AgentConfig.SetTrait(FAvoiding());

    UBattleFrameFunctionLibraryRT::SetRecordTeamTraitByIndex(FMath::Clamp(Team, 0, 9), AgentConfig);
    UBattleFrameFunctionLibraryRT::SetRecordSubTypeTraitByIndex(DataAsset->SubType.Index, AgentConfig);
    UBattleFrameFunctionLibraryRT::SetRecordAvoGroupTraitByIndex(FMath::Clamp(DataAsset->Avoidance.Group, 0, 9), AgentConfig);

    auto& HealthTrait = AgentConfig.GetTraitRef<FHealth>();
    auto& TextPopUp = AgentConfig.GetTraitRef<FTextPopUp>();
    auto& DamageTrait = AgentConfig.GetTraitRef<FDamage>();
    auto& ScaledTrait = AgentConfig.GetTraitRef<FScaled>();
    auto& MoveTrait = AgentConfig.GetTraitRef<FMove>();
    auto& ColliderTrait = AgentConfig.GetTraitRef<FCollider>();

    if (ColliderTrait.bHightQuality) AgentConfig.SetTrait(FRegisterMultiple{});

    HealthTrait.Current *= Multipliers.HealthMult;
    HealthTrait.Maximum *= Multipliers.HealthMult;
    TextPopUp.WhiteTextBelowPercent *= Multipliers.HealthMult;
    TextPopUp.OrangeTextAbovePercent *= Multipliers.HealthMult;
    DamageTrait.Damage *= Multipliers.DamageMult;
    ScaledTrait.Factors *= Multipliers.ScaleMult;
    ScaledTrait.renderFactors = ScaledTrait.Factors;
    MoveTrait.MoveSpeed *= Multipliers.SpeedMult;
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
        Located.preLocation = SpawnPoint3D;
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

            case EInitialDirection::FaceLocation:
            {
                FVector Delta = FaceCustomLocation - SpawnPoint3D;
                Directed.Direction = Delta.GetSafeNormal2D();
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

        Agent.GetTraitRef<FAvoiding, EParadigm::Unsafe>() = { SpawnPoint3D, Collider.Radius, Agent, Agent.CalcHash() };

        if (bAutoActivate)
        {
            ActivateAgent(Agent);
        }

        SpawnedAgents.Add(Agent);
    }

    return SpawnedAgents;
}

void AAgentSpawner::ActivateAgent( FSubjectHandle Agent )
{
    auto& Appear = Agent.GetTraitRef<FAppear, EParadigm::Unsafe>();
    auto& Sleep = Agent.GetTraitRef<FSleep, EParadigm::Unsafe>();
    auto& Patrol = Agent.GetTraitRef<FPatrol, EParadigm::Unsafe>();
    auto& Animation = Agent.GetTraitRef<FAnimation, EParadigm::Unsafe>();

    Animation.AnimToTextureData = Animation.AnimToTextureDataAsset.LoadSynchronous(); // DataAsset Solid Pointer

    if (IsValid(Animation.AnimToTextureData))
    {
        TArray<FAnimToTextureAnimInfo> Animations = Animation.AnimToTextureData->Animations;

        for (FAnimToTextureAnimInfo CurrentAnim : Animations)
        {
            Animation.AnimLengthArray.Add((CurrentAnim.EndFrame - CurrentAnim.StartFrame) / Animation.AnimToTextureData->SampleRate);
        }
    }

    if (Appear.bEnable)
    {
        Agent.SetTrait(FAppearing());
    }
    else
    {
        Animation.Dissolve = 0;
    }

    if (Sleep.bEnable)
    {
        Agent.SetTrait(FSleeping());
    }

    if (Patrol.bEnable)
    {
        Agent.SetTrait(FPatrolling());
    }

    Agent.SetTrait(FActivated());
}

void AAgentSpawner::KillAllAgents()
{
    const auto Mechanism = UMachine::ObtainMechanism(GetWorld());

    FFilter Filter = FFilter::Make<FAgent>();
    Filter.Exclude<FDying>();

    Mechanism->Operate<FUnsafeChain>(Filter,
    [&](FSubjectHandle Subject, FAgent& Agent)
    {
        Subject.SetTrait(FDying{ 0,0,FSubjectHandle{} });
    });
}

void AAgentSpawner::KillAgentsByIndex(int32 Index)
{
    const auto Mechanism = UMachine::ObtainMechanism(GetWorld());

    FFilter Filter = FFilter::Make<FAgent>().Exclude<FDying>();
    UBattleFrameFunctionLibraryRT::IncludeSubTypeTraitByIndex(Index, Filter);

    Mechanism->Operate<FUnsafeChain>(Filter,
    [&](FSubjectHandle Subject)
    {
        Subject.SetTraitDeferred(FDying{ 0,0,FSubjectHandle{} });
    });
}
