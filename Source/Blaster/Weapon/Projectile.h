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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Handles the projectile's behavior on impact */
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, FVector NormalImpulse,
		const FHitResult& HitResult);

	/** Damage done by the projectile */
	UPROPERTY(EditAnywhere)
	float Damage = 20.0f;

private:
	/** The collision bounding box for the projectile */
	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;

	/** Handles projectile movement */
	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;

	/** Tracer particle for the bullet */
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	/** Component to store the tracer in once spawned */
	UParticleSystemComponent* TracerComponent;

	/** Particles that play upon projectile impact */
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	/** Sound that plays upon projectile impact */
	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound;
};
