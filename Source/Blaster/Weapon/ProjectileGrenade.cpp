// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileGrenade.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileGrenade::AProjectileGrenade()
{
	// Initialize the mesh
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);

	// Initialize the projectile movement component
	ProjectileMovementComponent =
		CreateDefaultSubobject<UProjectileMovementComponent>(
			TEXT("ProjectileMovementComponent"));

	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenade::BeginPlay()
{
	// Skip AProjectile BeginPlay because we don't have a tracer
	AActor::BeginPlay();
	SpawnTrailSystem();
	// Grenades set their timer to destroy themselves immediately when spawned
	StartDestroyTimer();

	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(
		this, &AProjectileGrenade::OnBounce);
}

void AProjectileGrenade::OnBounce(
	const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	// Play the bounce sound
	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this, BounceSound, GetActorLocation());
	}
}

void AProjectileGrenade::Destroyed()
{
	// Apply radial damage
	ExplodeDamage();
	Super::Destroyed();
}
