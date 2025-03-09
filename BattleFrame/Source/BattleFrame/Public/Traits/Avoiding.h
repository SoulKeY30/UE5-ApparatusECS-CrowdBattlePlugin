#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "Avoiding.generated.h"

USTRUCT(BlueprintType, meta = (ForceAlignment = 8))
struct BATTLEFRAME_API FAvoiding
{
    GENERATED_BODY()

public:

    FVector Location{0,0,0};
    float Radius = 0;
    FSubjectHandle SubjectHandle = FSubjectHandle{};
    uint32 SubjectHash = 0;

    bool bValid = false;

    // 匹配Handle
    bool operator==(const FAvoiding& Other) const
    {
        return SubjectHandle == Other.SubjectHandle;
    }

    // 匹配Hash
    bool operator==(uint32 Hash) const
    {
        return SubjectHash == Hash;
    }

    // 查询Hash
    friend uint32 GetTypeHash(const FAvoiding& Data)
    {
        return Data.SubjectHash;
    }
};
