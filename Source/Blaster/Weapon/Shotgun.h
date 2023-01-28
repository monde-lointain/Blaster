// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitscanWeapon.h"

#include "Shotgun.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AShotgun : public AHitscanWeapon
{
	GENERATED_BODY()

public:
	void Fire(const FVector& HitTarget) override;

	/**
	 * The number of linetraces to perform for this shotgun, correponding to
	 * the number of pellets in the shell
	 */
	UPROPERTY(EditAnywhere)
	uint32 NumLinetraces = 10;
};
