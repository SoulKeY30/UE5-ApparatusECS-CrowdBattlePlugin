#pragma once

#include "CoreMinimal.h"
#include "SubjectHandle.h"
#include "BattleFrameStructs.generated.h" 

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FTraceResult
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FSubjectHandle Subject = FSubjectHandle();

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector Location = FVector::ZeroVector;

    float CachedDistSq = -1.0f;
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDmgResult
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FSubjectHandle DamagedSubject = FSubjectHandle();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FSubjectHandle InstigatorSubject = FSubjectHandle();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool IsCritical = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool IsKill = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float DmgDealt = 0;
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSubjectArray
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FSubjectHandle> Subjects = TArray<FSubjectHandle>();
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FSpawnerMult
{
	GENERATED_BODY()

public:

	// 成员变量默认值
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "血量乘数"))
	float HealthMult = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "速度乘数"))
	float MoveSpeedMult = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "伤害乘数"))
	float DamageMult = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "尺寸乘数"))
	float ScaleMult = 1.f;

	// 默认构造函数（必须）
	FSpawnerMult() = default;
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDebugPointConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "位置"))
	FVector Location = FVector::ZeroVector;

};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDebugLineConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "起点"))
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "终点"))
	FVector EndLocation = FVector::ZeroVector;

};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDebugSphereConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "半径"))
	float Radius = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "位置"))
	FVector Location = FVector::ZeroVector;

};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDebugCapsuleConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "半径"))
	float Radius = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "高度"))
	float Height = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "位置"))
	FVector Location = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDebugSectorConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "半径"))
	float Radius = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "高度"))
	float Height = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "张开角度"))
	float Angle = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "张开角度"))
	float Duration = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "位置"))
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "朝向"))
	FVector Direction = FVector::OneVector;
};

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FDebugCircleConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "半径"))
	float Radius = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Tooltip = "位置"))
	FVector Location = FVector::ZeroVector;

};

