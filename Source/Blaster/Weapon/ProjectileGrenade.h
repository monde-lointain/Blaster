// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"

#include "ProjectileGrenade.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	AProjectileGrenade();

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Handles behavior upon bouncing */
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	/** Played when the grenade bounces */
	UPROPERTY(EditAnywhere, Category = Sounds)
	USoundCue* BounceSound;

	/**
	 * Called when the actor has been destroyed. Damage is applied here for the
	 * grenade class
	 */
	virtual void Destroyed() override;
};
