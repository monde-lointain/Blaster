// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Projectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class UParticleSystemComponent;
class USoundCue;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	AProjectile();

	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	/**
	 * Called when the actor has been destroyed. Gets replicated down to
	 * clients when executed on the server.
	 */
	virtual void Destroyed() override;

	/** The mesh for the projectile */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	/**
	 * Damage done by the projectile. For rockets this is the maximum damage
	 * dealt when an actor is within the inner radius
	 */
	UPROPERTY(EditAnywhere, Category = Damage)
	float Damage = 20.0f;

	/** Handles projectile movement */
	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;

	/** The collision bounding box for the projectile */
	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;

	/** Tracer particle for the bullet */
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	/** Component to store the tracer in once spawned */
	UPROPERTY()
	UParticleSystemComponent* TracerComponent;

	/** Particles that play upon projectile impact */
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	/** Sound that plays upon projectile impact */
	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound;

	/** Particle system for the smoke trail particles */
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* TrailSystem;

	/** Component to store the trail particles in */
	UPROPERTY(EditAnywhere)
	UNiagaraComponent* TrailSystemComponent;

	void SpawnTrailSystem();

	/** Handle to the timer for destroying the projectile and smoke particles */
	FTimerHandle DestroyTimer;

	/**
	 * Time the projectile and smoke particles should persist after impact before
	 * being destroyed
	 */
	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.0f;

	/** Starts the destroy timer */
	void StartDestroyTimer();

	/** Destroys the projectile and smoke particles */
	void DestroyTimerFinished();

	/**
	 * (Radial damage projectiles only). The minimum damage dealt by the
	 * projectile
	 */
	UPROPERTY(EditDefaultsOnly, Category = RadialDamage)
	float MinimiumDamage = 5.0f;

	/**
	 * (Radial damage projectiles only). The radius in which maximum damage is
	 * dealt
	 */
	UPROPERTY(EditDefaultsOnly, Category = RadialDamage)
	float InnerRadius = 40.0f;

	/**
	 * (Radial damage projectiles only). The radius outside of which no damage
	 * is dealt
	 */
	UPROPERTY(EditDefaultsOnly, Category = RadialDamage)
	float OuterRadius = 500.0f;

	/**
	 * (Radial damage projectiles only). Exponent that gets applied to the
	 * damage falloff equation
	 */
	UPROPERTY(EditDefaultsOnly, Category = RadialDamage)
	float DamageFalloff = 1.5f;

	/** Handles dealing radial damage for exploding projectiles */
	void ExplodeDamage();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Handles the projectile's behavior on impact. */
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, FVector NormalImpulse,
		const FHitResult& HitResult);
};
