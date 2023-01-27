// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"

#include "ProjectileRocket.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
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

	/** The minimum damage dealt by the rocket */
	UPROPERTY(EditDefaultsOnly, Category = Damage)
	float MinimiumDamage = 5.0f;

	/** The radius in which maximum damage is dealt */
	UPROPERTY(EditDefaultsOnly, Category = Damage)
	float InnerRadius = 40.0f;

	/** The radius outside of which no damage is dealt */
	UPROPERTY(EditDefaultsOnly, Category = Damage)
	float OuterRadius = 500.0f;

	/** Exponent that gets applied to the damage falloff equation */
	UPROPERTY(EditDefaultsOnly, Category = Damage)
	float DamageFalloff = 1.5f;

	/** The mesh for the rocket */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;

	/** Sound that plays while the rocket is flying */
	UPROPERTY(EditAnywhere)
	USoundCue* RocketLoop;

	/** Component to store the rocket sounds in */
	UPROPERTY()
	UAudioComponent* RocketLoopComponent;

	/** Attenuation for the rocket loop */
	UPROPERTY(EditAnywhere)
	USoundAttenuation* RocketLoopAttenuation;

	/** Particle system for the smoke trail particles */
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* TrailSystem;

	/** Component to store the trail particles in */
	UPROPERTY(EditAnywhere)
	UNiagaraComponent* TrailSystemComponent;

	/** Handle to the timer for destroying rocket and smoke particles */
	FTimerHandle SmokePersistTimer;

	/**
	 * Time the rocket and smoke particles should persist after impact before
	 * being destroyed
	 */
	UPROPERTY(EditAnywhere)
	float SmokePersistTime = 3.0f;

	/** Destroys the rocket and smoke particles */
	void DestroyTimerFinished();

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
