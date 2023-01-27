// Fill out your copyright notice in the Description page of Project Settings.

#include "HitscanWeapon.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

void AHitscanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	// Get the owning player's controller
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	
	// If there's no owner then we can't do anything else
	if (!OwnerPawn)
	{
		return;
	}

	AController* InstigatorController = OwnerPawn->GetController();

	// Get the muzzle flash socket and use it as the starting point for the
	// linetrace
	const USkeletalMeshSocket* MuzzleFlashSocket =
		GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket && InstigatorController)
	{
		FTransform SocketTransform =
			MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		// Push the trace endpoint further back a bit so it's just slightly past
		// the object we're looking at
		FVector End = Start + (HitTarget - Start) * 1.25f;

		FHitResult FireHit;
		UWorld* World = GetWorld();

		if (World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);

			// If we hit anything
			if (FireHit.bBlockingHit)
			{
				ABlasterCharacter* BlasterCharacter =
					Cast<ABlasterCharacter>(FireHit.GetActor());

				// If we hit another player
				if (BlasterCharacter)
				{
					// Only done on the server!
					if (HasAuthority())
					{
						UGameplayStatics::ApplyDamage(
							BlasterCharacter,
							Damage,
							InstigatorController,
							this,
							UDamageType::StaticClass()
						);
					}

					if (ImpactParticles)
					{
						UGameplayStatics::SpawnEmitterAtLocation(
							World,
							ImpactParticles,
							FireHit.ImpactPoint,
							FireHit.ImpactNormal.Rotation()
						);
					}
				}
			}
		}
	}
}