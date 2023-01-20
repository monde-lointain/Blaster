// Fill out your copyright notice in the Description page of Project Settings.

#include "BulletShell.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
ABulletShell::ABulletShell()
{
	PrimaryActorTick.bCanEverTick = false;

	BulletMesh =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
	SetRootComponent(BulletMesh);

	BulletMesh->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	BulletMesh->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	BulletMesh->SetSimulatePhysics(true);
	BulletMesh->SetEnableGravity(true);
	BulletMesh->SetNotifyRigidBodyCollision(true);

	EjectionImpulseMagnitude = 500.0f;
	NumHits = 0;
}

// Called when the game starts or when spawned
void ABulletShell::BeginPlay()
{
	Super::BeginPlay();

	BulletMesh->OnComponentHit.AddDynamic(this, &ABulletShell::OnHit);
	BulletMesh->AddImpulse(GetActorForwardVector() * EjectionImpulseMagnitude);
}

void ABulletShell::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, FVector NormalImpulse,
	const FHitResult& HitResult)
{
	// Only play the impact sound for the first eight impacts
	if ((ShellSound) && (NumHits <= 8))
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ShellSound,
			GetActorLocation()
		);
	}

	// Set the despawn timer after the first impact
	if (NumHits == 0)
	{
		// Set the shell to despawn eight seconds after impacting
		float DespawnTime = 8.0f;

		GetWorldTimerManager().SetTimer(
			DespawnTimerHandle,
			this,
			&ABulletShell::DespawnProjectile,
			DespawnTime
		);
	}

	NumHits++;
}

void ABulletShell::DespawnProjectile()
{
	Destroy();
}
