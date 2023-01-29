// Fill out your copyright notice in the Description page of Project Settings.

#include "HitscanWeapon.h"

#include "WeaponTypes.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
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
		// Compute the start point of the linetrace (end with scatter gets
		// handled in HandleWeaponLineTrace)
		FTransform SocketTransform =
			MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		// Perform the linetrace
		FHitResult FireHit;
		HandleWeaponLineTrace(Start, HitTarget, FireHit);

		// Check to see if we've hit another player (only done on the server!)
		ABlasterCharacter* BlasterCharacter =
			Cast<ABlasterCharacter>(FireHit.GetActor());

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

		// Play particles and sound effects on both server and clients
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
	const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();

	if (World)
	{
		// Compute the end point of the trace if we're using scatter
		FVector TraceEnd = bUseScatter
							   ? TraceEndWithScatter(TraceStart, HitTarget)
							   : TraceStart + (HitTarget - TraceStart) * 1.25f;
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

FVector AHitscanWeapon::TraceEndWithScatter(
	const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	// Generate a random point within the sphere
	FVector RandVec = UKismetMathLibrary::RandomUnitVector()
					  * FMath::FRandRange(0.0f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVec;

	// Vector from start of linetrace to random endpoint
	FVector ToEndLoc = EndLoc - TraceStart;

	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.0f, 12, FColor::Orange, true);

	FVector Result(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
	DrawDebugLine(GetWorld(), TraceStart, Result, FColor::White, true);
	return Result;
}
