// Fill out your copyright notice in the Description page of Project Settings.

#include "Shotgun.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	// Skip the hitscan weapon's fire function
	AWeapon::Fire(HitTarget);

	// Get the owning player's controller. We don't need it right away but we
	// need to check here so we can exit out early if it is invalid, since we
	// can't do anything else otherwise.
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}
	AController* InstigatorController = OwnerPawn->GetController();

	// Get the muzzle flash socket and use it as the starting point for the
	// linetrace
	const USkeletalMeshSocket* MuzzleFlashSocket =
		GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket)
	{
		// Compute the starting and end points of our linetrace
		FTransform SocketTransform =
			MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		// Perform random scatter trace for each pellet in the shotgun
		for (uint32 i = 0; i < NumLinetraces; i++)
		{
			FVector End = TraceEndWithScatter(Start, HitTarget);
		}
	}
}
