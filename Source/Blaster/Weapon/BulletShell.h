// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "BulletShell.generated.h"

class USoundCue;

UCLASS()
class BLASTER_API ABulletShell : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	ABulletShell();

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Handles the shell's behavior on impact */
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, FVector NormalImpulse,
		const FHitResult& HitResult);

	/** 
	 * Despawns the projectile at the end of its despawn timer
	 */
	void DespawnProjectile();

private:
	/** Mesh for the bullet casing */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BulletMesh;

	/** Magnitude for the velocity at which the shell gets shot out */
	UPROPERTY(EditAnywhere)
	float EjectionImpulseMagnitude;

	/** Number of impacts a shell has made since spawning */
	int8 NumHits;

	/** Sound that plays upon shell impact */
	UPROPERTY(EditAnywhere)
	USoundCue* ShellSound;

	/** Handle for the projectile's despawn timer */
	FTimerHandle DespawnTimerHandle;
};
