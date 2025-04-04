/*
 * APPARATIST
 * Created: 2023-02-02 16:26:26
 * Author: Vladislav Dmitrievich Turbanov (vladislav@turbanov.ru)
 * Community forums: https://talk.turbanov.ru
 * Copyright 2019 - 2023, SP Vladislav Dmitrievich Turbanov
 * Made in Russia, Moscow City, Chekhov City
 */

 /*
  * BattleFrame
  * Refactor: 2025
  * Author: Leroy Works
  */

#pragma once

#include "CoreMinimal.h"
#include "Machine.h"
#include "Traits/Avoiding.h"
#include "NeighborGridCell.generated.h"
    
 /**
  * Struct representing a one grid cell.
  */
USTRUCT(BlueprintType, Category = "NeighborGrid")
struct BATTLEFRAME_API FNeighborGridCell
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

	FFingerprint SubjectFingerprint;
	FFingerprint SphereObstacleFingerprint;
	FFingerprint BoxObstacleFingerprint;
	FFingerprint SphereObstacleFingerprintStatic;
	FFingerprint BoxObstacleFingerprintStatic;

	TArray<FAvoiding> Subjects;
	TArray<FAvoiding> SphereObstacles;
	TArray<FAvoiding> BoxObstacles;
	TArray<FAvoiding> SphereObstaclesStatic;
	TArray<FAvoiding> BoxObstaclesStatic;


	FNeighborGridCell(){}

	FNeighborGridCell(const FNeighborGridCell& Cell)
	{
		LockFlag.store(Cell.LockFlag.load());

		SubjectFingerprint = Cell.SubjectFingerprint;
		SphereObstacleFingerprint = Cell.SphereObstacleFingerprint;
		BoxObstacleFingerprint = Cell.BoxObstacleFingerprint;
		SphereObstacleFingerprintStatic = Cell.SphereObstacleFingerprintStatic;
		BoxObstacleFingerprintStatic = Cell.BoxObstacleFingerprintStatic;
		Subjects = Cell.Subjects;
		SphereObstacles = Cell.SphereObstacles;
		BoxObstacles = Cell.BoxObstacles;
		SphereObstaclesStatic = Cell.SphereObstaclesStatic;
		BoxObstaclesStatic = Cell.BoxObstaclesStatic;
	}

	FNeighborGridCell& operator=(const FNeighborGridCell& Cell)
	{
		SubjectFingerprint = Cell.SubjectFingerprint;
		SphereObstacleFingerprint = Cell.SphereObstacleFingerprint;
		BoxObstacleFingerprint = Cell.BoxObstacleFingerprint;
		SphereObstacleFingerprintStatic = Cell.SphereObstacleFingerprintStatic;
		BoxObstacleFingerprintStatic = Cell.BoxObstacleFingerprintStatic;
		Subjects = Cell.Subjects;
		SphereObstacles = Cell.SphereObstacles;
		BoxObstacles = Cell.BoxObstacles;
		SphereObstaclesStatic = Cell.SphereObstaclesStatic;
		BoxObstaclesStatic = Cell.BoxObstaclesStatic;

		return *this;
	}

	void Reset()
	{
		// 重置所有指纹
		SubjectFingerprint.Reset();
		SphereObstacleFingerprint.Reset();
		BoxObstacleFingerprint.Reset();

		// 清空所有存储的障碍物数据
		Subjects.Reset();
		SphereObstacles.Reset();
		BoxObstacles.Reset();
	}
};
