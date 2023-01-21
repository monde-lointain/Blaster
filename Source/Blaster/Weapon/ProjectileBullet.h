// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"

#include "ProjectileBullet.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()

protected:
	/** Handles the projectile's behavior on impact */
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, FVector NormalImpulse,
		const FHitResult& HitResult) override;
};
