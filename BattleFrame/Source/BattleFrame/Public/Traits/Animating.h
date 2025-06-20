//#pragma once
//
//#include "CoreMinimal.h"
//#include "AnimToTextureDataAsset.h"
//#include "Animating.generated.h"
//
//UENUM(BlueprintType)
//enum class ESubjectState : uint8
//{
//    None UMETA(DisplayName = "None"),
//    Dirty UMETA(DisplayName = "Dirty"),
//    Appearing UMETA(DisplayName = "Appearing"),
//    Idle UMETA(DisplayName = "Idle"),
//    Moving UMETA(DisplayName = "Moving"),
//    Attacking UMETA(DisplayName = "Attacking"),
//    BeingHit UMETA(DisplayName = "BeingHit"),
//    Dying UMETA(DisplayName = "Dying"),
//    Falling UMETA(DisplayName = "Falling"),
//    Jumping UMETA(DisplayName = "Jumping"),
//    Sleeping UMETA(DisplayName = "Sleeping")
//};
//
//class ANiagaraSubjectRenderer;
//
//USTRUCT(BlueprintType)
//struct BATTLEFRAME_API FAnimating
//{
//    GENERATED_BODY()
//
//private:
//
//    mutable std::atomic<bool> LockFlag{ false };
//
//public:
//
//    void Lock() const
//    {
//        while (LockFlag.exchange(true, std::memory_order_acquire));
//    }
//
//    void Unlock() const
//    {
//        LockFlag.store(false, std::memory_order_release);
//    }
//
//    //-----------------------------------------------------------
//
//    UAnimToTextureDataAsset* AnimToTextureData;
//
//    //-----------------------------------------------------------
//
//    TArray<float> AnimLengthArray;
//
//    //-----------------------------------------------------------
//
//    float AnimIndex0 = 0;
//    float AnimPlayRate0 = 1.f;
//    float AnimCurrentTime0 = 0.f;
//    float AnimOffsetTime0 = 0.f;
//    float AnimPauseTime0 = 0.f;
//
//    float AnimIndex1 = 0;
//    float AnimPlayRate1 = 1.f;
//    float AnimCurrentTime1 = 0.f;
//    float AnimOffsetTime1 = 0.f;
//    float AnimPauseTime1 = 0.f;
//
//    float AnimLerp = 1;
//
//    //-----------------------------------------------------------
//
//    float HitGlow = 0;
//    float Dissolve = 0;
//    float Team = 0;
//
//    float IceFx = 0;
//    float FireFx = 0;
//    float PoisonFx = 0;
//
//    //-----------------------------------------------------------
//
//    ESubjectState SubjectState = ESubjectState::None;
//    ESubjectState PreviousSubjectState = ESubjectState::Dirty;
//
//    //-----------------------------------------------------------
//
//    FAnimating() {};
//
//    FAnimating(const FAnimating& Anim)
//    {
//        LockFlag.store(Anim.LockFlag.load());
//
//        HitGlow = Anim.HitGlow;
//        Dissolve = Anim.Dissolve;
//        Team = Anim.Team;
//
//        IceFx = Anim.IceFx;
//        FireFx = Anim.FireFx;
//        PoisonFx = Anim.PoisonFx;
//
//        AnimIndex0 = Anim.AnimIndex0;
//        AnimPlayRate0 = Anim.AnimPlayRate0;
//        AnimCurrentTime0 = Anim.AnimCurrentTime0;
//        AnimOffsetTime0 = Anim.AnimOffsetTime0;
//        AnimPauseTime0 = Anim.AnimPauseTime0;
//
//        AnimIndex1 = Anim.AnimIndex1;
//        AnimPlayRate1 = Anim.AnimPlayRate1;
//        AnimCurrentTime1 = Anim.AnimCurrentTime1;
//        AnimOffsetTime1 = Anim.AnimOffsetTime1;
//        AnimPauseTime1 = Anim.AnimPauseTime1;
//
//        AnimLerp = Anim.AnimLerp;
//
//        AnimLengthArray = Anim.AnimLengthArray;
//
//        SubjectState = Anim.SubjectState;
//        PreviousSubjectState = Anim.PreviousSubjectState;
//    }
//
//    FAnimating& operator=(const FAnimating& Anim)
//    {
//        LockFlag.store(Anim.LockFlag.load());
//
//        HitGlow = Anim.HitGlow;
//        Dissolve = Anim.Dissolve;
//        Team = Anim.Team;
//
//        IceFx = Anim.IceFx;
//        FireFx = Anim.FireFx;
//        PoisonFx = Anim.PoisonFx;
//
//        AnimIndex0 = Anim.AnimIndex0;
//        AnimPlayRate0 = Anim.AnimPlayRate0;
//        AnimCurrentTime0 = Anim.AnimCurrentTime0;
//        AnimOffsetTime0 = Anim.AnimOffsetTime0;
//        AnimPauseTime0 = Anim.AnimPauseTime0;
//
//        AnimIndex1 = Anim.AnimIndex1;
//        AnimPlayRate1 = Anim.AnimPlayRate1;
//        AnimCurrentTime1 = Anim.AnimCurrentTime1;
//        AnimOffsetTime1 = Anim.AnimOffsetTime1;
//        AnimPauseTime1 = Anim.AnimPauseTime1;
//
//        AnimLerp = Anim.AnimLerp;
//
//        AnimLengthArray = Anim.AnimLengthArray;
//
//        SubjectState = Anim.SubjectState;
//        PreviousSubjectState = Anim.PreviousSubjectState;
//
//        return *this;
//    }
//
//};
