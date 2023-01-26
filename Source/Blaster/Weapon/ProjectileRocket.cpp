// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	// Initialize the mesh
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);

	// Disable collision
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse, const FHitResult& HitResult)
{
	APawn* InstigatorPawn = GetInstigator();

	if (InstigatorPawn)
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

			DrawDebugSphere(
				GetWorld(),
				GetActorLocation(),
				InnerRadius,
				12,
				FColor::Red,
				true,
				10.0f
			);
			DrawDebugSphere(
				GetWorld(),
				GetActorLocation(),
				OuterRadius,
				12,
				FColor::Yellow,
				true,
				10.0f
			);
		}
	}

	// Destroy
	Super::OnHit(
		HitComponent, OtherActor, OtherComponent, NormalImpulse, HitResult);
}