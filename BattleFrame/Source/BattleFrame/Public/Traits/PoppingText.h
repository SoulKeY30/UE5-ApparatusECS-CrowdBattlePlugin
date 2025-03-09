#pragma once

#include "CoreMinimal.h"
#include <atomic>

#include "PoppingText.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FPoppingText
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

public:

    TArray<FVector> TextLocationArray;
    TArray<FVector4> Text_Value_Style_Scale_Offset_Array; // 4 in one

    FPoppingText() {};

    FPoppingText(const FPoppingText& PoppingText)
    {
        LockFlag.store(PoppingText.LockFlag.load());

        TextLocationArray = PoppingText.TextLocationArray;
        Text_Value_Style_Scale_Offset_Array = PoppingText.Text_Value_Style_Scale_Offset_Array;
    }

    FPoppingText& operator=(const FPoppingText& PoppingText)
    {
        LockFlag.store(PoppingText.LockFlag.load());

        TextLocationArray = PoppingText.TextLocationArray;
        Text_Value_Style_Scale_Offset_Array = PoppingText.Text_Value_Style_Scale_Offset_Array;

        return *this;
    }

};
