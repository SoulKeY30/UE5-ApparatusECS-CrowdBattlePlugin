#include "BFSubjectiveAgentComponent.h"
#include "Traits/Health.h"
#include "Traits/GridData.h"
#include "Traits/Located.h"
#include "Traits/Directed.h"
#include "Traits/Scaled.h"
#include "Traits/Collider.h"
#include "Traits/BindFlowField.h"
#include "Traits/Activated.h"
#include "Traits/IsSubjective.h"
#include "Traits/TemporalDamaging.h"
#include "Traits/Slowing.h"
#include "Traits/Damage.h"
#include "Traits/SubType.h"
#include "Traits/Agent.h"
#include "Traits/Moving.h"
#include "Traits/Appearing.h"
#include "Traits/Move.h"
#include "Traits/Dying.h"
#include "Traits/TextPopUp.h"
#include "Traits/Animation.h"
#include "Traits/Appear.h"
#include "Traits/Tracing.h"
#include "Traits/RegisterMultiple.h"
#include "Traits/Team.h"
#include "Traits/Avoiding.h"
#include "Traits/PoppingText.h"
#include "Traits/Sleeping.h"
#include "Traits/Patrolling.h"
#include "BattleFrameFunctionLibraryRT.h"
#include "NiagaraSubjectRenderer.h"

UBFSubjectiveAgentComponent::UBFSubjectiveAgentComponent()
{
    // Enable ticking
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UBFSubjectiveAgentComponent::BeginPlay()
{
    Super::BeginPlay();

    if(bAutoInitWithDataAsset) InitializeTraits(GetOwner());
}

void UBFSubjectiveAgentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bSyncTransformSubjectToActor) SyncTransformSubjectToActor(GetOwner());
}

void UBFSubjectiveAgentComponent::InitializeTraits(AActor* OwnerActor)
{
    if (!OwnerActor) return;

    if (!CurrentWorld)
    {
        CurrentWorld = GetWorld();

        if (!CurrentWorld) return;
    }

    if (!Mechanism)
    {
        Mechanism = UMachine::ObtainMechanism(CurrentWorld);

        if (!Mechanism) return;
    }

    if (!BattleControl)
    {
        BattleControl = Cast<ABattleFrameBattleControl>(UGameplayStatics::GetActorOfClass(CurrentWorld, ABattleFrameBattleControl::StaticClass()));

        if (!BattleControl) return;
    }

    UAgentConfigDataAsset* DataAsset = AgentConfigAsset.LoadSynchronous();

    if (!IsValid(DataAsset)) return;

    FSubjectRecord AgentConfig = DataAsset->ExtraTraits;

    AgentConfig.SetTrait(DataAsset->Agent);
    AgentConfig.SetTrait(DataAsset->SubType);
    AgentConfig.SetTrait(FTeam(TeamIndex));
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
    AgentConfig.SetTrait(FAvoiding());
    AgentConfig.SetTrait(DataAsset->Appear);
    AgentConfig.SetTrait(DataAsset->Trace);
    AgentConfig.SetTrait(FTracing());
    AgentConfig.SetTrait(DataAsset->Chase);
    AgentConfig.SetTrait(DataAsset->Attack);
    AgentConfig.SetTrait(DataAsset->Hit);
    AgentConfig.SetTrait(DataAsset->Death);
    AgentConfig.SetTrait(DataAsset->Animation);
    AgentConfig.SetTrait(DataAsset->HealthBar);
    AgentConfig.SetTrait(DataAsset->TextPop);
    AgentConfig.SetTrait(FPoppingText());
    AgentConfig.SetTrait(DataAsset->Curves);
    AgentConfig.SetTrait(FTemporalDamaging());
    AgentConfig.SetTrait(FSlowing());
    AgentConfig.SetTrait(DataAsset->Statistics);
    AgentConfig.SetTrait(FIsSubjective());
    AgentConfig.SetTrait(FActivated());

    // Apply Multipliers
    auto& Health = AgentConfig.GetTraitRef<FHealth>();
    Health.Current *= Multipliers.HealthMult;
    Health.Maximum *= Multipliers.HealthMult;

    auto& TextPopUp = AgentConfig.GetTraitRef<FTextPopUp>();
    TextPopUp.WhiteTextBelowPercent *= Multipliers.HealthMult;
    TextPopUp.OrangeTextAbovePercent *= Multipliers.HealthMult;

    auto& Damage = AgentConfig.GetTraitRef<FDamage>();
    Damage.Damage *= Multipliers.DamageMult;

    auto& Scaled = AgentConfig.GetTraitRef<FScaled>();
    Scaled.Scale *= Multipliers.ScaleMult;
    Scaled.RenderScale *= Multipliers.ScaleMult;

    auto& Move = AgentConfig.GetTraitRef<FMove>();
    Move.XY.MoveSpeed *= Multipliers.MoveSpeedMult;

    auto& Collider = AgentConfig.GetTraitRef<FCollider>();
    auto& Located = AgentConfig.GetTraitRef<FLocated>();
    auto& Directed = AgentConfig.GetTraitRef<FDirected>();
    auto& Moving = AgentConfig.GetTraitRef<FMoving>();
    auto& Patrol = AgentConfig.GetTraitRef<FPatrol>();

    Directed.Direction = OwnerActor->GetActorRotation().Vector().GetSafeNormal2D();

    FVector SpawnPoint3D = OwnerActor->GetActorLocation();

    if (Move.Z.bCanFly)
    {
        Moving.FlyingHeight = FMath::RandRange(Move.Z.FlyHeightRange.X, Move.Z.FlyHeightRange.Y);
        SpawnPoint3D = FVector(SpawnPoint3D.X, SpawnPoint3D.Y, SpawnPoint3D.Z + Moving.FlyingHeight);
    }

    Patrol.Origin = SpawnPoint3D;
    Moving.Goal = SpawnPoint3D;

    Located.Location = SpawnPoint3D;
    Located.PreLocation = SpawnPoint3D;
    Located.InitialLocation = SpawnPoint3D;

    if (LaunchVelocity.Size() > 0)
    {
        Moving.LaunchVelSum = Directed.Direction * LaunchVelocity.X + Directed.Direction.UpVector * LaunchVelocity.Y;
        Moving.bLaunching = true;
    }

    auto& Appear = AgentConfig.GetTraitRef<FAppear>();
    auto& Sleep = AgentConfig.GetTraitRef<FSleep>();
    auto& Animation = AgentConfig.GetTraitRef<FAnimation>();
    auto& Team = AgentConfig.GetTraitRef<FTeam>();
    auto& SubType = AgentConfig.GetTraitRef<FSubType>();
    auto& Avoidance = AgentConfig.GetTraitRef<FAvoidance>();

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
        AgentConfig.SetTrait(FAppearing());
    }

    Animation.AnimOffsetTime0 = FMath::RandRange(Animation.IdleRandomTimeOffset.X, Animation.IdleRandomTimeOffset.Y);
    Animation.AnimOffsetTime1 = FMath::RandRange(Animation.IdleRandomTimeOffset.X, Animation.IdleRandomTimeOffset.Y);

    if (Sleep.bEnable)
    {
        AgentConfig.SetTrait(FSleeping());
    }

    if (Patrol.bEnable)
    {
        AgentConfig.SetTrait(FPatrolling());
    }

    if (Collider.bHightQuality)
    {
        //Agent.SetTrait(FRegisterMultiple());
        AgentConfig.SetFlag(RegisterMultipleFlag, true);
    }

    UBattleFrameFunctionLibraryRT::SetRecordSubTypeTraitByIndex(SubType.Index, AgentConfig);
    UBattleFrameFunctionLibraryRT::SetRecordTeamTraitByIndex(FMath::Clamp(Team.index, 0, 9), AgentConfig);
    UBattleFrameFunctionLibraryRT::SetRecordAvoGroupTraitByIndex(FMath::Clamp(Avoidance.Group, 0, 9), AgentConfig);

    AgentConfig.SetTrait(FGridData{ this->GetHandle().CalcHash(), FVector3f(Located.Location), Collider.Radius * Scaled.Scale, this->GetHandle() });

    this->GetHandle()->SetTraits(AgentConfig);

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
}

void UBFSubjectiveAgentComponent::SyncTransformSubjectToActor(AActor* OwnerActor)
{
    if (!OwnerActor) return;

    FTransform Transform;

    auto Located = GetTraitPtr<FLocated, EParadigm::Unsafe>();

    if (Located)
    {
        Transform.SetLocation(Located->Location);
    }

    auto Directed = GetTraitPtr<FDirected, EParadigm::Unsafe>();

    if (Directed)
    {
        Transform.SetRotation(Directed->Direction.ToOrientationQuat());
    }

    auto Scaled = GetTraitPtr<FScaled, EParadigm::Unsafe>();

    if (Scaled)
    {
        Transform.SetScale3D(FVector(Scaled->Scale));
    }

    OwnerActor->SetActorTransform(Transform);
}
