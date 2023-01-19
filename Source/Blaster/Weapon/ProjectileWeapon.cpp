// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileWeapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	// When we fire a projectile we only want to spawn it on the server. We can
	// still spawn the particle effects, but as far as the actual particle
	// object goes, we want to give the server as much authority as possible in
	// order to prevent cheating
	if (!HasAuthority())
	{
		return;
	}

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	// Get the socket to spawn the projectile at
	const USkeletalMeshSocket* MuzzleFlashSocket =
		GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	if (MuzzleFlashSocket)
	{
		// Get the location to spawn the projectile at from the socket
		FVector SocketTransformLocation =
			MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh())
				.GetLocation();

		// Calculate the rotation of the projectile as pointing in the direction
		// from the projectile spawn point to the projectile's hit target
		// (calculated in TraceUnderCrosshairs)
		FVector ToTarget = HitTarget - SocketTransformLocation;
		FRotator TargetRotation = ToTarget.Rotation();

		if (ProjectileClass && InstigatorPawn)
		{
			// Set the spawn parameters for the projectile
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;

			// Spawn the projectile at the target location
			UWorld* World = GetWorld();

			if (World)
			{
				World->SpawnActor<AProjectile>(
					ProjectileClass, 
					SocketTransformLocation, 
					TargetRotation, 
					SpawnParams
					);
			}
		}
	}
}
