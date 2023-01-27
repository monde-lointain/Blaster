// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileBullet.h"

#include "Kismet/GameplayStatics.h"
#include "Gameframework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileBullet::AProjectileBullet()
{
	// Initialize the projectile movement component
	ProjectileMovementComponent =
		CreateDefaultSubobject<UProjectileMovementComponent>(
			TEXT("ProjectileMovementComponent"));

	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse, const FHitResult& HitResult)
{
	// Before applying damage, check to see if the owner and the owner's
	// controller are both valid
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (OwnerCharacter)
	{
		AController* OwnerController = OwnerCharacter->Controller;

		if (OwnerController)
		{
			// Only triggers the take damage event. The actual damage handling
			// is done in ABlasterCharacter::ReceiveDamage
			UGameplayStatics::ApplyDamage(
				OtherActor, 
				Damage, 
				OwnerController, 
				this, 
				UDamageType::StaticClass()
			);
		}
	}
	Super::OnHit(HitComponent, OtherActor, OtherComponent, NormalImpulse, HitResult);
}
