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
		// Compute the start point of the linetrace (end with scatter gets
		// handled in HandleWeaponLineTrace)
		FTransform SocketTransform =
			MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		// Assign the number of hits for each character to a map
		TMap<ABlasterCharacter*, uint32> HitMap;

		for (uint32 i = 0; i < NumLinetraces; i++)
		{
			// Perform a linetrace with random scatter for each pellet in the
			// shotgun
			FHitResult FireHit;
			HandleWeaponLineTrace(Start, HitTarget, FireHit);

			// Check to see if we've hit another player (only on the server)
			ABlasterCharacter* BlasterCharacter =
				Cast<ABlasterCharacter>(FireHit.GetActor());

			// Check to see who was hit with the pellet and add a hit to their
			// slot on the hitmap, adding new characters to the hit map when
			// they get hit for the first time
			if (BlasterCharacter && HasAuthority() && InstigatorController)
			{
				if (HitMap.Contains(BlasterCharacter))
				{
					HitMap[BlasterCharacter]++;
				}
				else
				{
					HitMap.Emplace(BlasterCharacter, 1);
				}
			}

			// Play the impact effects for each pellet
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}
			if (ImpactSound)
			{
				float VolumeMultiplier = 0.5f;
				float PitchMultiplier = FMath::FRandRange(-0.5f, 0.5f);

				UGameplayStatics::PlaySoundAtLocation(
					this,
					ImpactSound,
					FireHit.ImpactPoint,
					VolumeMultiplier,
					PitchMultiplier
				);
			}
		}

		// Parse the hit results
		for (auto& HitPair : HitMap)
		{
			ABlasterCharacter* BlasterCharacter = HitPair.Key;

			if (BlasterCharacter && HasAuthority() && InstigatorController)
			{
				uint32 NumHits = HitPair.Value;
				float TotalDamage = Damage * NumHits;

				UGameplayStatics::ApplyDamage(
					BlasterCharacter,
					TotalDamage,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}
}
