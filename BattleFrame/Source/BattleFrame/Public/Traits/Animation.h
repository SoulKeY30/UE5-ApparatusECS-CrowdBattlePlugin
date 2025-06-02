#pragma once
 
#include "CoreMinimal.h"
#include "AnimToTextureDataAsset.h"
#include "Animation.generated.h"
 
UENUM(BlueprintType)
enum class ESubjectState : uint8
{
    None UMETA(DisplayName = "None"),
    Dirty UMETA(DisplayName = "Dirty"),
    Appearing UMETA(DisplayName = "Appearing"),
    Idle UMETA(DisplayName = "Idle"),
    Moving UMETA(DisplayName = "Moving"),
    Attacking UMETA(DisplayName = "Attacking"),
    BeingHit UMETA(DisplayName = "BeingHit"),
    Dying UMETA(DisplayName = "Dying"),
    Falling UMETA(DisplayName = "Falling"),
    Jumping UMETA(DisplayName = "Jumping"),
    Sleeping UMETA(DisplayName = "Sleeping")
};

class ANiagaraSubjectRenderer;

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FAnimation
{
	GENERATED_BODY()

private:

    mutable std::atomic<bool> LockFlag{ false };

public:

    void Lock() const
    {
        while (LockFlag.exchange(true, std::memory_order_acquire));
    }

    void Unlock() const
    {
        LockFlag.store(false, std::memory_order_release);
    }

    //-----------------------------------------------------------

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TSoftObjectPtr <UAnimToTextureDataAsset> AnimToTextureDataAsset;

    UAnimToTextureDataAsset* AnimToTextureData;

    //-----------------------------------------------------------

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TSubclassOf<ANiagaraSubjectRenderer> RendererClass;

    //-------------------------------------------------------------

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "动画间过度速度"))
    float LerpSpeed = 4;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "待机动画播放速度"))
    float IdlePlayRate = 1;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "移动动画播放速度"))
    float MovePlayRate = 1;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "出生动画的索引值"))
    int32 IndexOfAppearAnim = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "攻击动画的索引值"))
    int32 IndexOfAttackAnim = 1;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "死亡动画的索引值"))
    int32 IndexOfDeathAnim = 2;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "待机动画的索引值"))
    int32 IndexOfIdleAnim = 3;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "移动动画的索引值"))
    int32 IndexOfMoveAnim = 4;

    //-----------------------------------------------------------

    TArray<float> AnimLengthArray;

    //-----------------------------------------------------------

    float AnimIndex0 = IndexOfIdleAnim;
    float AnimPlayRate0 = 1.f;
    float AnimCurrentTime0 = 0.f;
    float AnimOffsetTime0 = 0.f;
    float AnimPauseTime0 = 0.f;

    float AnimIndex1 = IndexOfIdleAnim;
    float AnimPlayRate1 = 1.f;
    float AnimCurrentTime1 = 0.f;
    float AnimOffsetTime1 = 0.f;
    float AnimPauseTime1 = 0.f;

    float AnimLerp = 1;

    //-----------------------------------------------------------

    float HitGlow = 0;
    float Dissolve = 0;
    float Team = 0;

    float IceFx = 0;
    float FireFx = 0;
    float PoisonFx = 0;

    //-----------------------------------------------------------

    ESubjectState SubjectState = ESubjectState::None;
    ESubjectState PreviousSubjectState = ESubjectState::Dirty;

    //-----------------------------------------------------------

    FAnimation() {};

    FAnimation(const FAnimation& Anim)
    {
        LockFlag.store(Anim.LockFlag.load());

        AnimToTextureDataAsset = Anim.AnimToTextureDataAsset;
        AnimToTextureData = Anim.AnimToTextureData;

        RendererClass = Anim.RendererClass;

        HitGlow = Anim.HitGlow;
        Dissolve = Anim.Dissolve;
        Team = Anim.Team;

        IceFx = Anim.IceFx;
        FireFx = Anim.FireFx;
        PoisonFx = Anim.PoisonFx;

        AnimIndex0 = Anim.AnimIndex0;
        AnimPlayRate0 = Anim.AnimPlayRate0;
        AnimCurrentTime0 = Anim.AnimCurrentTime0;
        AnimOffsetTime0 = Anim.AnimOffsetTime0;
        AnimPauseTime0 = Anim.AnimPauseTime0;

        AnimIndex1 = Anim.AnimIndex1;
        AnimPlayRate1 = Anim.AnimPlayRate1;
        AnimCurrentTime1 = Anim.AnimCurrentTime1;
        AnimOffsetTime1 = Anim.AnimOffsetTime1;
        AnimPauseTime1 = Anim.AnimPauseTime1;

        AnimLerp = Anim.AnimLerp;
        LerpSpeed = Anim.LerpSpeed;

        AnimLengthArray = Anim.AnimLengthArray;

        IndexOfAppearAnim = Anim.IndexOfAppearAnim;
        IndexOfIdleAnim = Anim.IndexOfIdleAnim;
        IndexOfMoveAnim = Anim.IndexOfMoveAnim;
        IndexOfAttackAnim = Anim.IndexOfAttackAnim;
        IndexOfDeathAnim = Anim.IndexOfDeathAnim;

        IdlePlayRate = Anim.IdlePlayRate;
        MovePlayRate = Anim.MovePlayRate;

        SubjectState = Anim.SubjectState;
        PreviousSubjectState = Anim.PreviousSubjectState;
    }

    FAnimation& operator=(const FAnimation& Anim)
    {
        LockFlag.store(Anim.LockFlag.load());

        AnimToTextureDataAsset = Anim.AnimToTextureDataAsset;
        AnimToTextureData = Anim.AnimToTextureData;

        RendererClass = Anim.RendererClass;

        HitGlow = Anim.HitGlow;
        Dissolve = Anim.Dissolve;
        Team = Anim.Team;

        IceFx = Anim.IceFx;
        FireFx = Anim.FireFx;
        PoisonFx = Anim.PoisonFx;

        AnimIndex0 = Anim.AnimIndex0;
        AnimPlayRate0 = Anim.AnimPlayRate0;
        AnimCurrentTime0 = Anim.AnimCurrentTime0;
        AnimOffsetTime0 = Anim.AnimOffsetTime0;
        AnimPauseTime0 = Anim.AnimPauseTime0;

        AnimIndex1 = Anim.AnimIndex1;
        AnimPlayRate1 = Anim.AnimPlayRate1;
        AnimCurrentTime1 = Anim.AnimCurrentTime1;
        AnimOffsetTime1 = Anim.AnimOffsetTime1;
        AnimPauseTime1 = Anim.AnimPauseTime1;

        AnimLerp = Anim.AnimLerp;
        LerpSpeed = Anim.LerpSpeed;

        AnimLengthArray = Anim.AnimLengthArray;

        IndexOfAppearAnim = Anim.IndexOfAppearAnim;
        IndexOfIdleAnim = Anim.IndexOfIdleAnim;
        IndexOfMoveAnim = Anim.IndexOfMoveAnim;
        IndexOfAttackAnim = Anim.IndexOfAttackAnim;
        IndexOfDeathAnim = Anim.IndexOfDeathAnim;

        IdlePlayRate = Anim.IdlePlayRate;
        MovePlayRate = Anim.MovePlayRate;

        SubjectState = Anim.SubjectState;
        PreviousSubjectState = Anim.PreviousSubjectState;

        return *this;
    }

};
