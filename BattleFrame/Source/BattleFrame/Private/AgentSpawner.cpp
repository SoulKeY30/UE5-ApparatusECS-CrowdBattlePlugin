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
#include "AnimToTextureDataAsset.h"
#include "NiagaraSubjectRenderer.h"
#include "BattleFrameFunctionLibraryRT.h"


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

    // 明确使用ANiagaraSubjectRenderer类型
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
    AgentConfig.SetTrait(DataAsset->Move);
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
    AgentConfig.SetTrait(DataAsset->FX);
    AgentConfig.SetTrait(DataAsset->Sound);
    AgentConfig.SetTrait(DataAsset->SpawnActor);
    AgentConfig.SetTrait(DataAsset->Curves);
    //AgentConfig.SetTrait(DataAsset->Statistics);

    AgentConfig.SetTrait(FTracing{});

    UBattleFrameFunctionLibraryRT::SetSubTypeTraitByIndex(DataAsset->SubType.Index, AgentConfig);

    auto& SourceAnimation = AgentConfig.GetTraitRef<FAnimation>();
    SourceAnimation.AnimToTextureData = SourceAnimation.AnimToTextureDataAsset.LoadSynchronous(); // DataAsset Solid Pointer

    if (!IsValid(SourceAnimation.AnimToTextureData)) return SpawnedAgents;

    TArray<FAnimToTextureAnimInfo> Animations = SourceAnimation.AnimToTextureData->Animations;
    int32 AnimLastIndex = Animations.Num() - 1;

    if (SourceAnimation.IndexOfAppearAnim <= AnimLastIndex)
    {
        SourceAnimation.AppearAnimLength = (SourceAnimation.AnimToTextureData->Animations[SourceAnimation.IndexOfAppearAnim].EndFrame - SourceAnimation.AnimToTextureData->Animations[SourceAnimation.IndexOfAppearAnim].StartFrame) / SourceAnimation.AnimToTextureData->SampleRate;
    }

    if (SourceAnimation.IndexOfAttackAnim <= AnimLastIndex)
    {
        SourceAnimation.AttackAnimLength = (SourceAnimation.AnimToTextureData->Animations[SourceAnimation.IndexOfAttackAnim].EndFrame - SourceAnimation.AnimToTextureData->Animations[SourceAnimation.IndexOfAttackAnim].StartFrame) / SourceAnimation.AnimToTextureData->SampleRate;
    }

    if (SourceAnimation.IndexOfDeathAnim <= AnimLastIndex)
    {
        SourceAnimation.DeathAnimLength = (SourceAnimation.AnimToTextureData->Animations[SourceAnimation.IndexOfDeathAnim].EndFrame - SourceAnimation.AnimToTextureData->Animations[SourceAnimation.IndexOfDeathAnim].StartFrame) / SourceAnimation.AnimToTextureData->SampleRate;
    }

    auto& HealthTrait = AgentConfig.GetTraitRef<FHealth>();
    HealthTrait.Current *= Multipliers.HealthMult;
    HealthTrait.Maximum *= Multipliers.HealthMult;

    auto& TextPopUp = AgentConfig.GetTraitRef<FTextPopUp>();
    TextPopUp.WhiteTextBelowPercent *= Multipliers.HealthMult;
    TextPopUp.OrangeTextAbovePercent *= Multipliers.HealthMult;

    auto& Damage = AgentConfig.GetTraitRef<FDamage>();
    Damage.Damage *= Multipliers.DamageMult;

    auto& ScaledTrait = AgentConfig.GetTraitRef<FScaled>();
    ScaledTrait.Factors *= Multipliers.ScaleMult;
    ScaledTrait.renderFactors = ScaledTrait.Factors;

    auto& Appear = AgentConfig.GetTraitRef<FAppear>();
    auto& Animation = AgentConfig.GetTraitRef<FAnimation>();

    if (Appear.bEnable)
    {
        AgentConfig.SetTrait(FAppearing{});
    }
    else
    {
        Animation.Dissolve = 0;
    }

    UBattleFrameFunctionLibraryRT::SetTeamTraitByIndex(FMath::Clamp(Team, 0, 9), AgentConfig);

    UBattleFrameFunctionLibraryRT::SetAvoGroupTraitByIndex(FMath::Clamp(DataAsset->Avoidance.Group, 0, 9), AgentConfig);

    while (SpawnedAgents.Num() < Quantity)// the following traits varies from agent to agent
    {
        FSubjectRecord Config = AgentConfig;

        auto& Collider = Config.GetTraitRef<FCollider>();
        Collider.Radius *= Multipliers.ScaleMult;

        if (Collider.bHightQuality)
        {
            Config.SetTrait(FRegisterMultiple{});
        }

        FVector SpawnPoint2D;

        float RandomX = FMath::RandRange(-Region.X / 2, Region.X / 2);
        float RandomY = FMath::RandRange(-Region.Y / 2, Region.Y / 2);
        SpawnPoint2D = Origin + FVector(RandomX, RandomY, 0);

        FVector SpawnPoint3D;
        auto& Move = Config.GetTraitRef<FMove>();

        Config.SetTrait(FMoving{});
        auto& Moving = Config.GetTraitRef<FMoving>();

        if (Move.bCanFly)
        {
            Moving.FlyingHeight = FMath::RandRange(Move.FlyHeightRange.X, Move.FlyHeightRange.Y);
            SpawnPoint3D = FVector(SpawnPoint2D.X, SpawnPoint2D.Y, Moving.FlyingHeight + GetActorLocation().Z);
        }
        else
        {
            SpawnPoint3D = FVector(SpawnPoint2D.X, SpawnPoint2D.Y, Collider.Radius + GetActorLocation().Z);
        }

        FLocated spawnLocated;
        spawnLocated.Location = SpawnPoint3D;
        spawnLocated.preLocation = SpawnPoint3D;
        Config.SetTrait(spawnLocated);

        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

        FVector Direction;

        switch (InitialDirection)
        {
            case EInitialDirection::FacePlayer:
            {
                if (IsValid(PlayerPawn))
                {
                    FVector Delta = PlayerPawn->GetActorLocation() - SpawnPoint3D;
                    Direction = Delta.GetSafeNormal2D();
                }
                else
                {
                    Direction = GetActorForwardVector().GetSafeNormal2D();
                }
                break;
            }

            case EInitialDirection::FaceForward:
            {
                Direction = GetActorForwardVector().GetSafeNormal2D();
                break;
            }

            case EInitialDirection::FaceLocation:
            {
                FVector Delta = FaceCustomLocation - SpawnPoint3D;
                Direction = Delta.GetSafeNormal2D();
                break;
            }
        }

        Config.SetTrait(FDirected{ Direction });

        Move.MoveSpeed *= Multipliers.SpeedMult;

        if (LaunchForce.Size() > 0)
        {         
            Moving.LaunchForce = Direction * LaunchForce.X + Direction.UpVector * LaunchForce.Y;
            Moving.bLaunching = true;
        }

        // Spawn using the modified record
        const auto Agent = Mechanism->SpawnSubject(Config);

        Agent.SetTrait(FAvoiding{ SpawnPoint3D, Collider.Radius, Agent, Agent.CalcHash()});

        SpawnedAgents.Add(Agent);
    }

    return SpawnedAgents;
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

    FFilter Filter = FFilter::Make<FAgent, FHealth>().Exclude<FDying>();
    UBattleFrameFunctionLibraryRT::IncludeSubTypeTraitByIndex(Index, Filter);

    Mechanism->Operate<FUnsafeChain>(Filter,
    [&](FSubjectHandle Subject)
    {
        Subject.SetTraitDeferred(FDying{ 0,0,FSubjectHandle{} });
    });
}
