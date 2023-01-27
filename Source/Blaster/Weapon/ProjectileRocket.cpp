// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileRocket.h"

#include "RocketMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileRocket::AProjectileRocket()
{
	// Initialize the mesh
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);

	// Disable collision
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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
	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}

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

	APawn* InstigatorPawn = GetInstigator();

	// Apply damage only on the server
	if (InstigatorPawn && HasAuthority())
	{
		AController* InstigatorController = InstigatorPawn->GetController();

		if (InstigatorController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, 
				Damage,
				MinimiumDamage,
				GetActorLocation(),
				InnerRadius,
				OuterRadius,
				DamageFalloff,
				UDamageType::StaticClass(),
				TArray<AActor*>(),
				this,
				InstigatorController
			);

			//DrawDebugSphere(
			//	GetWorld(),
			//	GetActorLocation(),
			//	InnerRadius,
			//	12,
			//	FColor::Red,
			//	true,
			//	10.0f
			//);
			//DrawDebugSphere(
			//	GetWorld(),
			//	GetActorLocation(),
			//	OuterRadius,
			//	12,
			//	FColor::Yellow,
			//	true,
			//	10.0f
			//);
		}
	}

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
	if (RocketMesh)
	{
		RocketMesh->SetVisibility(false);
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
	GetWorldTimerManager().SetTimer(
		SmokePersistTimer,
		this,
		&AProjectileRocket::DestroyTimerFinished,
		SmokePersistTime
	);
}

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}

void AProjectileRocket::Destroyed()
{
}