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

#include "Located.generated.h"


/**
 * Struct representing actor/projectile that have a position in the world.
 */
USTRUCT(BlueprintType, Category = "Basic")
struct BATTLEFRAME_API FLocated
{
	GENERATED_BODY()

  public:

	/* 当前位置 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	FVector Location = FVector::ZeroVector;

	/* 前一帧位置 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	FVector preLocation = FVector::ZeroVector; 

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	//FVector nextLocation = FVector::ZeroVector;


	/** Initialize a default instance. */
	FORCEINLINE
	FLocated()
	{}

	/** Initialize with a vector. */
	FORCEINLINE
	FLocated(const FVector& InLocation)
	  : Location(InLocation)
	{}

	/* Simple get Location pure function. */
	FORCEINLINE const FVector&
	GetLocation() const
	{
		return Location;
	}

	/* Simple set Location pure function. */
	FORCEINLINE void
	SetLocation(const FVector& InLocation)
	{
		Location = InLocation;
	}

	/**
	 * Implicit conversion to an immutable vector.
	 */
	FORCEINLINE
	operator const FVector&() const
	{
		return Location;
	}

	/**
	 * Implicit conversion to a mutable vector.
	 */
	FORCEINLINE
	operator FVector&()
	{
		return Location;
	}
};
