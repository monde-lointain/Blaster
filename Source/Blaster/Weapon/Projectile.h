// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Projectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class UParticleSystemComponent;

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
