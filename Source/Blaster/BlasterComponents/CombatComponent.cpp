// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatComponent.h"

#include "Blaster/Character/BlasterAnimInstance.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"

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

	// Register the ammo carried by the player to be replicated by the server.
	// Only replicate for the owner
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);

	// Register the combat state to be replicated by the server
	DOREPLIFETIME(UCombatComponent, CombatState);
}

void UCombatComponent::ShotgunShellReload()
{
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoCounts();
	}
}

void UCombatComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();

	if (AnimInstance && Character->ReloadMontage)
	{
		AnimInstance->Montage_Play(Character->ReloadMontage);
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
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

		// We only want the server to have control over the amount of ammo we're
		// carrying
		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
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
	if (Character == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}

	bIsAiming = bAiming;
	ServerSetAiming(bAiming);

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed =
			bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}

	// Play the zoom animation effects when zooming in and out with the sniper
	// rifle
	if (Character->IsLocallyControlled() &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		if (!Controller)
		{
			Controller = Cast<ABlasterPlayerController>(Character->Controller);
		}
		
		if (Controller)
		{
			// Bring up the HUD widget
			Controller->SetHUDSniperScope(bIsAiming);

			// Play the sound effects
			if (bIsAiming)
			{
				UGameplayStatics::PlaySound2D(this, ZoomInSniperRifle);
			}
			else
			{
				UGameplayStatics::PlaySound2D(this, ZoomOutSniperRifle);
			}
		}
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

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		// Call the server RPC for firing a weapon.
		ServerFire(HitTarget);

		// Make the crosshairs spread apart when firing
		if (EquippedWeapon)
		{
			CrosshairShootFactor = 0.75f;
		}

		StartFireTimer();
	}
}

void UCombatComponent::StartFireTimer()
{
	if (!EquippedWeapon || !Character)
	{
		return;
	}

	bCanFire = false;

	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (!EquippedWeapon)
	{
		return;
	}

	bCanFire = true;

	// Fire again if we're still holding down the fire button
	if (bFireButtonPressed && EquippedWeapon->bIsAutomatic)
	{
		Fire();
	}

	// Autoreload if the weapon gets emptied
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

bool UCombatComponent::CanFire()
{
	if (!EquippedWeapon)
	{
		return false;
	}

	// Special handling for shotguns
	if (!EquippedWeapon->IsEmpty() &&
		bCanFire &&
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
	{
		return true;
	}

	return !EquippedWeapon->IsEmpty() && 
		   bCanFire &&
		   CombatState == ECombatState::ECS_Unoccupied;
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
	if (EquippedWeapon == nullptr)
	{
		return;
	}

	// For shotguns we CAN fire while reloading
	if (Character && 
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
	{ 
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
		// Manually set combat state back to unoccupied since we won't always
		// get to the anim notify at the end of the reload animation
		CombatState = ECombatState::ECS_Unoccupied;
		return;
	}

	// ONLY fire again if we're not reloading, otherwise the animation will get
	// stuck
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ThrowGrenade()
{
	// Don't throw a grenade unless we're not doing anything
	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		return;
	}

	// Change the combat state
	CombatState = ECombatState::ECS_ThrowingGrenade;

	// Play the grenade montage
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
	}

	// Only call for the server RPC here if we ARE the server
	if (Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	// Change the combat state
	CombatState = ECombatState::ECS_ThrowingGrenade;

	// Play the grenade montage
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
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
					  (CrosshairWorldDirection * TRACE_LENGTH);

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
	// We're only allowed to equip weapons if we're not doing anything else
	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		return;
	}
	// If we're already holding a weapon, drop it and pick up the new one
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}

	// Replicated from server to clients
	EquippedWeapon = WeaponToEquip;

	UpdateWeaponStateAndAttach();

	// Automatically replicated by the actor class method AActor::OnRep_Owner
	EquippedWeapon->SetOwner(Character);

	// Update the weapon ammo counter on the HUD
	EquippedWeapon->SetAmmoCountOnOwnerHUD();

	// Set the carried ammo amount on the player to the amount they have on them
	// for that particular weapon type
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	// Update the carried ammo counter on the HUD
	if (!Controller)
	{
		Controller = Cast<ABlasterPlayerController>(Character->Controller);
	}
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	// Play the weapon equip sound
	if (EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this, 
			EquippedWeapon->EquipSound, 
			Character->GetActorLocation()
		);
	}
	
	// Autoreload if the weapon is empty when it gets picked up
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::Reload()
{
	/**
	 * If we're on a client, ask the server if it's okay to reload, before
	 * letting the server relay that it's time to replay the reload animation
	 * for all clients
	 */

	// Only message the server if we actually have ammo to save bandwidth. Only
	// notify the server if we're not doing anything else
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied)
	{
		ServerReload();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	// Change the combat state, which will be replicated down to clients
	CombatState = ECombatState::ECS_Reloading;
	// Reload
	HandleReload();
}

// NOTE: Plays at the end of the reload animation (specifically at the anim
// notify ReloadFinish in the reload montage)
void UCombatComponent::FinishReloading()
{
	if (!Character)
	{
		return;
	}
	// Reset the weapon state and add ammo into the weapon
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoCounts();
	}
	// Fire again if we're still holding down the button
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::UpdateAmmoCounts()
{
	if (!Character || !EquippedWeapon)
	{
		return;
	}

	int32 ReloadAmount = CalculateReloadAmmo();

	// Update the carried ammo counters for that particular weapon
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	// Update the carried ammo counter on the HUD
	if (!Controller)
	{
		Controller = Cast<ABlasterPlayerController>(Character->Controller);
	}
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	// Add the ammo into the weapon
	EquippedWeapon->AddAmmo(ReloadAmount);
}

void UCombatComponent::UpdateShotgunAmmoCounts()
{
	if (!Character || !EquippedWeapon)
	{
		return;
	}

	// Update the carried ammo counters for the shotgun. Only load in one shell at a time
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	// Update the carried ammo counter on the HUD
	if (!Controller)
	{
		Controller = Cast<ABlasterPlayerController>(Character->Controller);
	}
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	// Add the ammo into the weapon
	EquippedWeapon->AddAmmo(1);

	bCanFire = true;

	// Jump to the end montage when full or when empty (only on server)
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	    case (ECombatState::ECS_Unoccupied):
		{
			if (bFireButtonPressed)
			{
				Fire();
			}
			break;
		}
	    case (ECombatState::ECS_Reloading):
		{
			HandleReload();
			break;
		}
		case (ECombatState::ECS_ThrowingGrenade):
		{
			// Only play the montage here if the character is not locally
			// controlled, since the montage would've been played already
			// otherwise
			if (Character && !Character->IsLocallyControlled())
			{
				Character->PlayThrowGrenadeMontage();
			}
			break;
		}
	}
}

void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
}

int32 UCombatComponent::CalculateReloadAmmo()
{
	if (!EquippedWeapon)
	{
		return 0;
	}

	int32 AmountEmpty =
		EquippedWeapon->GetMaxAmmo() - EquippedWeapon->GetCurrentAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		// Only reload enough ammo to fill the weapon to max capacity
		int32 Least = FMath::Min(AmountEmpty, AmountCarried);
		// Clamp in case the current ammo is set higher than the max ammo for
		// whatever reason
		return FMath::Clamp(AmountEmpty, 0, Least);
	}

	// Return zero if we try to add ammo to a weapon type that isn't in the map
	return 0;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (Character && EquippedWeapon)
	{
		// The server automatically replicates the weapon state, but we're
		// making sure to do it again here because if physics doesn't get
		// disabled in time then the character won't be able to equip the weapon
		UpdateWeaponStateAndAttach();
		
		// Play the weapon equip sound
		if (EquippedWeapon->EquipSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				EquippedWeapon->EquipSound,
				Character->GetActorLocation()
			);
		}

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

// NOTE: Automatically propagated to clients when called on the server
void UCombatComponent::UpdateWeaponStateAndAttach()
{
	// Update the weapon state
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	// Attach the weapon to the player's right hand socket
	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	const USkeletalMeshSocket* HandSocket =
		CharacterMesh->GetSocketByName("RightHandSocket");

	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, CharacterMesh);
	}
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	// Update the carried ammo counter on the HUD
	if (!Controller)
	{
		Controller = Cast<ABlasterPlayerController>(Character->Controller);
	}
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	bool bShouldJumpToShotgunEnd =
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
		CarriedAmmo == 0;

	if (bShouldJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	// Initialize the ammo for all the different weapon types
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, AssaultRifleStartAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, RocketLauncherStartAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, PistolStartAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, SubmachineGunStartAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, ShotgunStartAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, SniperRifleStartAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, GrenadeLauncherStartAmmo);
}
