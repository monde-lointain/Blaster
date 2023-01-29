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

	/**
	 * Simulates random scatter by creating a sphere at the hit target and
	 * performing multiple linetraces to random points within the volume of the
	 * sphere
	 */
	FVector TraceEndWithScatter(
		const FVector& TraceStart, const FVector& HitTarget);

	void HandleWeaponLineTrace(
		const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	/** The damage dealt by the weapon */
	UPROPERTY(EditAnywhere)
	float Damage = 20.0f;

	/** Represents the range of the scatter weapon */
	UPROPERTY(EditAnywhere, Category = WeaponScatter)
	float DistanceToSphere = 3000.0f;

	/** The radius of the sphere to perform the scatter in */
	UPROPERTY(EditAnywhere, Category = WeaponScatter)
	float SphereRadius = 1000.0f;

	/** Enables random scatter for the weapon */
	UPROPERTY(EditAnywhere, Category = WeaponScatter)
	bool bUseScatter = false;

	/** Particles spawned by the weapon upon impact */
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	/** Particles for the beam particle effects appearing behind the bullet */
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	/** Particles for the muzzle flash */
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	/** Sound that plays upon firing */
	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;

	/** Sound that plays upon impact */
	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound;
};
