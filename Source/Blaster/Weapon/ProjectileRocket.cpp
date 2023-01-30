// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileRocket.h"

#include "RocketMovementComponent.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileRocket::AProjectileRocket()
{
	// Initialize the mesh
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);

	// Disable collision
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Initialize the rocket movement component
	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(
		TEXT("RocketMovementComponent"));

	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	// Since we already have OnHit bound on the server in Projectile, we want to
	// bind OnHit to the clients as well for the rocket launcher, since we're
	// playing the particles and sound in OnHit and the clients need to see that
	// too
	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(
			this, &AProjectileRocket::OnHit);
	}

	// Spawn the particle effects
	SpawnTrailSystem();

	// Spawn the sound with its attenuation
	if (RocketLoop && RocketLoopAttenuation)
	{
		RocketLoopComponent = UGameplayStatics::SpawnSoundAttached(
			RocketLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.0f,
			1.0f,
			0.0f,
			RocketLoopAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
	}
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse, const FHitResult& HitResult)
{
	// Ignore the owner actor
	if (OtherActor == GetOwner())
	{
		return;
	}

	// Deal radial damage
	ExplodeDamage();

	// Play the impact sounds and particle effects
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ImpactParticles,
			GetActorTransform()
		);
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ImpactSound,
			GetActorLocation()
		);
	}

	// Hide the rocket mesh
	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}

	// Disable collision
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Stop spawning particles
	if (TrailSystemComponent &&
		TrailSystemComponent->GetSystemInstanceController())
	{
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();
	}

	// Stop playing the rocket sounds
	if (RocketLoopComponent && RocketLoopComponent->IsPlaying())
	{
		RocketLoopComponent->Stop();
	}

	// Set the timer for the missile to destroy itself
	StartDestroyTimer();
}

void AProjectileRocket::Destroyed()
{
}