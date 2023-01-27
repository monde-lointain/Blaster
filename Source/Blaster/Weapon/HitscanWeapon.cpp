// Fill out your copyright notice in the Description page of Project Settings.

#include "HitscanWeapon.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AHitscanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

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
		// Push the trace endpoint further back a bit so it's just slightly past
		// the object we're looking at
		FVector End = Start + (HitTarget - Start) * 1.25f;

		// Perform the linetrace
		FHitResult FireHit;
		HandleWeaponLineTrace(Start, End, FireHit);

		// Check to see if we've hit another player
		ABlasterCharacter* BlasterCharacter =
			Cast<ABlasterCharacter>(FireHit.GetActor());

		// If we hit another character (only done on the server!)
		if (BlasterCharacter && HasAuthority() && InstigatorController)
		{
			UGameplayStatics::ApplyDamage(
				BlasterCharacter,
				Damage,
				InstigatorController,
				this,
				UDamageType::StaticClass()
			);
		}

		// Play particles and sound effects on all clients
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash, 
				SocketTransform
			);
		}
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
			);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
		if (ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				ImpactSound,
				FireHit.ImpactPoint
			);
		}
	}
}

void AHitscanWeapon::HandleWeaponLineTrace(
	const FVector& TraceStart, const FVector& TraceEnd, FHitResult& OutHit)
{
	UWorld* World = GetWorld();

	if (World)
	{
		World->LineTraceSingleByChannel(
			OutHit, 
			TraceStart, 
			TraceEnd,
			ECollisionChannel::ECC_Visibility
		);

		// Set the particle beam to either the end of the linetrace, or the
		// impact point of the hit if we hit something
		FVector BeamEnd = TraceEnd;

		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			OutHit.ImpactPoint = TraceEnd;
		}

		// Spawn the beam particles regardless of whether we hit anything or not
		if (BeamParticles)
		{
			// Store so we can set the endpoint
			UParticleSystemComponent* Beam =
				UGameplayStatics::SpawnEmitterAtLocation(
					World, 
					BeamParticles,
					TraceStart, 
					FRotator::ZeroRotator, 
					true
				);

			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}
