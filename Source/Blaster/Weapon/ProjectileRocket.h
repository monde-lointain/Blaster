// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"

#include "ProjectileRocket.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();

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

protected:
	/** Handles the projectile's behavior on impact */
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, FVector NormalImpulse,
		const FHitResult& HitResult) override;
};
