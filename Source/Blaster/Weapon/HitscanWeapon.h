// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"

#include "HitscanWeapon.generated.h"

class UParticleSystem;

/**
 *
 */
UCLASS()
class BLASTER_API AHitscanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	/** Handles elements of firing assigned to the weapon */
	virtual void Fire(const FVector& HitTarget);

	/** The damage dealt by the weapon */
	UPROPERTY(EditAnywhere)
	float Damage = 20.0f;

	/** Particles spawned by the weapon upon impact */
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;
};
