#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2DDynamic.h"
#include "Statistics.generated.h"

USTRUCT(BlueprintType)
struct BATTLEFRAME_API FStatistics
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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "启用"))
	bool bEnable = true;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere) 
	//FString PlayerID = "null";  // Changed from int32 to FString

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	//FString PlayerName = "null";

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	//UTexture2DDynamic* PlayerProfileImage = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "总伤害"))
	float totalDamage = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "总击杀"))
	int32 totalKills = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "总分值"))
	int32 totalScore = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "总存活时长"))
	float totalTime = 0;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	//TArray<FSubjectHandle> bossFightHandle;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	//TArray<float> bossFightDamage;

	FStatistics() {};

	FStatistics(const FStatistics& Statistics)
    {
        LockFlag.store(Statistics.LockFlag.load());

		bEnable = Statistics.bEnable;
		//PlayerID = Statistics.PlayerID;
		//PlayerName = Statistics.PlayerName;
		//PlayerProfileImage = Statistics.PlayerProfileImage;
		totalDamage = Statistics.totalDamage;
		totalKills = Statistics.totalKills;
		totalScore = Statistics.totalScore;
		totalTime = Statistics.totalTime;
		//bossFightHandle = Statistics.bossFightHandle;
		//bossFightDamage = Statistics.bossFightDamage;
    }

	FStatistics& operator=(const FStatistics& Statistics)
    {
        LockFlag.store(Statistics.LockFlag.load());

		bEnable = Statistics.bEnable;
		//PlayerID = Statistics.PlayerID;
		//PlayerName = Statistics.PlayerName;
		//PlayerProfileImage = Statistics.PlayerProfileImage;
		totalDamage = Statistics.totalDamage;
		totalKills = Statistics.totalKills;
		totalScore = Statistics.totalScore;
		totalTime = Statistics.totalTime;
		//bossFightHandle = Statistics.bossFightHandle;
		//bossFightDamage = Statistics.bossFightDamage;

        return *this;
    }
};
