// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"

#include "ProjectileRocket.generated.h"

class URocketMovementComponent;

/**
 *
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();

	/** Handles movement for the rocket */
	UPROPERTY(VisibleAnywhere)
	URocketMovementComponent* RocketMovementComponent;

	/** Sound that plays while the rocket is flying */
	UPROPERTY(EditAnywhere)
	USoundCue* RocketLoop;

	/** Component to store the rocket sounds in */
	UPROPERTY()
	UAudioComponent* RocketLoopComponent;

	/** Attenuation for the rocket loop */
	UPROPERTY(EditAnywhere)
	USoundAttenuation* RocketLoopAttenuation;

	/**
	 * Overridden to not see the explosion or sound when this object is
	 * destroyed. We play those in OnHit instead
	 */
	void Destroyed();

protected:
	virtual void BeginPlay() override;

	/**
	 * Handles the projectile's behavior on impact. For the rocket launcher we
	 * are playing the impact sounds and particle effects and then setting a
	 * timer so that the smoke trail can persist before being despawned
	 */
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, FVector NormalImpulse,
		const FHitResult& HitResult) override;
};
