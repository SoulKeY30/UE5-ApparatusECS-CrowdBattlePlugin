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
    if(!AgentConfigAssets.IsValidIndex(ConfigIndex)) return SpawnedAgents;

    UAgentConfigDataAsset* DataAsset = AgentConfigAssets[ConfigIndex].LoadSynchronous();
    if (!IsValid(DataAsset)) return SpawnedAgents;

    FSubjectRecord AgentConfig;

    AgentConfig.SetTrait(DataAsset->Agent);
    AgentConfig.SetTrait(DataAsset->SubType);
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

    // 补一下省略的Traits
    AgentConfig.SetTrait(FLocated());
    AgentConfig.SetTrait(FTracing());
    AgentConfig.SetTrait(FMoving());
    AgentConfig.SetTrait(FDirected());
    AgentConfig.SetTrait(FTeam(Team));
    AgentConfig.SetTrait(FAvoiding());

    // 乘数
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

    auto& ColliderTrait = AgentConfig.GetTraitRef<FCollider>();
    ColliderTrait.Radius *= Multipliers.ScaleMult;

    auto& MoveTrait = AgentConfig.GetTraitRef<FMove>();
    MoveTrait.MoveSpeed *= Multipliers.SpeedMult;

    // the following traits varies from agent to agent
    while (SpawnedAgents.Num() < Quantity)
    {
        FSubjectRecord Config = AgentConfig;

        auto& Move = Config.GetTraitRef<FMove>();
        auto& Moving = Config.GetTraitRef<FMoving>();
        auto& Located = Config.GetTraitRef<FLocated>();
        auto& Patrol = Config.GetTraitRef<FPatrol>();
        auto& Directed = Config.GetTraitRef<FDirected>();

        // Spawn Location
        FVector SpawnPoint2D;
        FVector SpawnPoint3D;

        float RandomX = FMath::RandRange(-Region.X / 2, Region.X / 2);
        float RandomY = FMath::RandRange(-Region.Y / 2, Region.Y / 2);

        SpawnPoint2D = Origin + FVector(RandomX, RandomY, 0);

        if (Move.bCanFly)// Spawn in mid air
        {
            Moving.FlyingHeight = FMath::RandRange(Move.FlyHeightRange.X, Move.FlyHeightRange.Y) + GetActorLocation().Z;
            SpawnPoint3D = FVector(SpawnPoint2D.X, SpawnPoint2D.Y, Moving.FlyingHeight);
        }
        else
        {
            SpawnPoint3D = FVector(SpawnPoint2D.X, SpawnPoint2D.Y, ColliderTrait.Radius + GetActorLocation().Z);
        }

        Located.Location = SpawnPoint3D;
        Located.preLocation = SpawnPoint3D;
        Located.InitialLocation = SpawnPoint3D;

        Patrol.Origin = SpawnPoint3D;
        Move.Goal = SpawnPoint3D;

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
    auto& Located = Agent.GetTraitRef<FLocated, EParadigm::Unsafe>();
    auto& Collider = Agent.GetTraitRef<FCollider, EParadigm::Unsafe>();
    auto& SubType = Agent.GetTraitRef<FSubType, EParadigm::Unsafe>();
    auto& Team = Agent.GetTraitRef<FTeam, EParadigm::Unsafe>();
    auto& Avoidance = Agent.GetTraitRef<FAvoidance, EParadigm::Unsafe>();
    auto& Avoiding = Agent.GetTraitRef<FAvoiding, EParadigm::Unsafe>();

    Avoiding = { Located.Location, Collider.Radius, Agent, Agent.CalcHash() };

    // Spawn renderer if none
    if (Animation.RendererClass)
    {     
        AActor* Renderer = UGameplayStatics::GetActorOfClass(GetWorld(), Animation.RendererClass);

        if (!Renderer)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            SpawnParams.bNoFail = true;

            // 直接生成目标类型
            ANiagaraSubjectRenderer* NewRenderer = GetWorld()->SpawnActor<ANiagaraSubjectRenderer>(Animation.RendererClass, FTransform::Identity, SpawnParams);

            // 设置SubType参数
            if (NewRenderer)
            {
                NewRenderer->SubType.Index = SubType.Index;
                NewRenderer->TickEnabled = true;
            }
        }
    }

    // Cache anim length
    Animation.AnimToTextureData = Animation.AnimToTextureDataAsset.LoadSynchronous(); // DataAsset Solid Pointer

    if (IsValid(Animation.AnimToTextureData))
    {
        TArray<FAnimToTextureAnimInfo> Animations = Animation.AnimToTextureData->Animations;
        int32 AnimLastIndex = Animations.Num() - 1;

        if (Animation.IndexOfAppearAnim <= AnimLastIndex)
        {
            Animation.AppearAnimLength = (Animation.AnimToTextureData->Animations[Animation.IndexOfAppearAnim].EndFrame - Animation.AnimToTextureData->Animations[Animation.IndexOfAppearAnim].StartFrame) / Animation.AnimToTextureData->SampleRate;
        }

        if (Animation.IndexOfAttackAnim <= AnimLastIndex)
        {
            Animation.AttackAnimLength = (Animation.AnimToTextureData->Animations[Animation.IndexOfAttackAnim].EndFrame - Animation.AnimToTextureData->Animations[Animation.IndexOfAttackAnim].StartFrame) / Animation.AnimToTextureData->SampleRate;
        }

        if (Animation.IndexOfDeathAnim <= AnimLastIndex)
        {
            Animation.DeathAnimLength = (Animation.AnimToTextureData->Animations[Animation.IndexOfDeathAnim].EndFrame - Animation.AnimToTextureData->Animations[Animation.IndexOfDeathAnim].StartFrame) / Animation.AnimToTextureData->SampleRate;
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

    if (Collider.bHightQuality)
    {
        Agent.SetTrait(FRegisterMultiple{});
    }

    UBattleFrameFunctionLibraryRT::SetSubjectSubTypeTraitByIndex(SubType.Index, Agent);
    UBattleFrameFunctionLibraryRT::SetSubjectTeamTraitByIndex(FMath::Clamp(Team.index, 0, 9), Agent);
    UBattleFrameFunctionLibraryRT::SetSubjectAvoGroupTraitByIndex(FMath::Clamp(Avoidance.Group, 0, 9), Agent);

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
