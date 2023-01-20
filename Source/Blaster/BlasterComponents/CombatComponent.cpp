// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"

#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 1000.0f;
	AimWalkSpeed = 750.0f;
}

void UCombatComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Register the equipped weapon to be replicated by the server
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);

	// Register the aiming flag to be replicated by the server
	DOREPLIFETIME(UCombatComponent, bIsAiming);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		// Set the FOV
		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/** For the local player only */
	if (Character && Character->IsLocallyControlled())
	{
		SetHUDCrosshairs(DeltaTime);

		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (!Character || !Character->Controller)
	{
		return;
	}

	Controller = Controller == nullptr
					 ? Cast<ABlasterPlayerController>(Character->Controller)
					 : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;

		// Set the crosshairs textures for the HUD to draw
		if (HUD)
		{
			// Only set the crosshairs textures if we have a weapon equipped
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft   = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight  = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop    = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft   = nullptr;
				HUDPackage.CrosshairsRight  = nullptr;
				HUDPackage.CrosshairsTop    = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}

			// Calculate the crosshair spread based on the player's velocity
			UCharacterMovementComponent* CharacterMovement =
				Character->GetCharacterMovement();

			FVector2D WalkSpeedRange(0.0f, CharacterMovement->MaxWalkSpeed);
			FVector2D VelocityMappingRange(0.0f, 1.0f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.0f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
				WalkSpeedRange, VelocityMappingRange, Velocity.Size());

			// Calculate the in air factor based on whether the player is
			// falling or not
			if (CharacterMovement->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(
					CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				// Interpolate to zero when we hit the ground
				CrosshairInAirFactor = FMath::FInterpTo(
					CrosshairInAirFactor, 0.0f, DeltaTime, 30.0f);
			}

			// Calculate the aiming factor based on whether the player is aiming
			// or not
			if (bIsAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(
					CrosshairAimFactor, 0.55f, DeltaTime, 30.0f);
			}
			else
			{
				CrosshairAimFactor =
					FMath::FInterpTo(CrosshairAimFactor, 0.0f, DeltaTime, 30.0f);
			}

			CrosshairShootFactor =
				FMath::FInterpTo(CrosshairShootFactor, 0.0f, DeltaTime, 10.0f);

			HUDPackage.CrosshairSpread = 0.5f 
				                         + CrosshairVelocityFactor
										 + CrosshairInAirFactor
										 - CrosshairAimFactor
				                         + CrosshairShootFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (!EquippedWeapon)
	{
		return;
	}

	// Set the FOV based on whether we're aiming or not. The FOV and
	// interpolation speed are dependent on the weapon
	if (bIsAiming)
	{
		CurrentFOV = FMath::FInterpTo(
			CurrentFOV,
			EquippedWeapon->GetZoomedFOV(),
			DeltaTime,
			EquippedWeapon->GetZoomInterpSpeed()
		);
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(
			CurrentFOV,
			DefaultFOV,
			DeltaTime,
			ZoomInterpSpeed
		);
	}

	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetAiming(bool bAiming)
{
	bIsAiming = bAiming;
	ServerSetAiming(bAiming);

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed =
			bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bIsAiming = bAiming;

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed =
			bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (Character && EquippedWeapon)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	// Calculate the hit result, then call the server RPC for firing a weapon.
	if (bFireButtonPressed)
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		ServerFire(HitResult.ImpactPoint);

		if (EquippedWeapon)
		{
			CrosshairShootFactor = 0.75f;
		}
	}
}


// When a client calls this server RPC, the server will execute its multicast
// RPC which will replicate the fire routines back down to the clients
void UCombatComponent::ServerFire_Implementation(
	const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

// Plays on all clients and the server
void UCombatComponent::MulticastFire_Implementation(
	const FVector_NetQuantize& TraceHitTarget)
{
	if (!EquippedWeapon)
	{
		return;
	}

	if (Character)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);

	// Get the world coordinates of the crosshairs from the 2D crosshair
	// location
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	bool bDeprojectionSuccessful = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	// Perform a line trace starting from the center of the screen
	if (bDeprojectionSuccessful)
	{
		const float LINETRACE_LENGTH = 80000.0f;
		FVector Start = CrosshairWorldPosition;

		// Push the start of the linetrace forward just beyond our character
		if (Character)
		{
			float DistanceToCharacter =
				(Character->GetActorLocation() - Start).Size();
			float SpacingBeyondCharacter = 100.0f;
			Start += CrosshairWorldDirection *
					 (DistanceToCharacter + SpacingBeyondCharacter);

			//DrawDebugSphere(
			//	GetWorld(),
			//	Start,
			//	20.0f,
			//	8,
			//	FColor::Red
			//);
		}

		FVector End = CrosshairWorldPosition +
					  (CrosshairWorldDirection * LINETRACE_LENGTH);

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult, 
			Start, 
			End, 
			ECollisionChannel::ECC_Visibility
		);

		//DrawDebugSphere(
		//	GetWorld(),
		//	TraceHitResult.ImpactPoint,
		//	20.0f,
		//	8,
		//	FColor::Red
		//);

		// Set the crosshairs color based on whether we're aiming the weapon at
		// an actor that has crosshairs interaction implemented
		if (TraceHitResult.GetActor() &&
			TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
	}
}

// NOTE: Called by the server only. For the variable replication implementation
// for clients, see AWeapon::OnRep_WeaponState (in Blaster/Weapon/Weapon.h)
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!Character || !WeaponToEquip)
	{
		return;
	}

	// Replicated from server to clients
	EquippedWeapon = WeaponToEquip;

	// Set the weapon state for the server. Replicated to clients in
	// AWeapon::OnRep_WeaponState (in Blaster/Weapon/Weapon.h)
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	const USkeletalMeshSocket* HandSocket =
		CharacterMesh->GetSocketByName("RightHandSocket");

	// Attach the weapon to the player's right hand socket. Automatically
	// propagated from server to clients
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, CharacterMesh);
	}

	// Automatically replicated by the actor class method AActor::OnRep_Owner
	EquippedWeapon->SetOwner(Character);

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}
