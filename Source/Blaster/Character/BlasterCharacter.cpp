// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterCharacter.h"

#include "Blaster/Blaster.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "BlasterAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to
	// improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize the spring arm component for the camera
	CameraBoom =
		CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Initialize the camera component
	FollowCamera =
		CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Initialize the overhead widget displaying the player's name and their net
	// role
	OverheadWidget =
		CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	// Initialize the combat component
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	// Initialize the timeline component for the dissolve effect
	DissolveTimeline =
		CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeline"));

	// Initialize the grenade mesh
	AttachedGrenade =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachedGrenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// Enable crouching
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// Set the collision properties for the capsule component
	GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// Set the collision properties for the character mesh
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	// Set the amount the character can rotate when moving
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 600.0f);

	// Initialize the character's turning in place state
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	// Set this so we can make sure the actor always spawns even if there aren't
	// enough spawn points
	SpawnCollisionHandlingMethod =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Set the net update frequency for the character
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void ABlasterCharacter::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Register the overlapping weapon to be replicated by the server
	DOREPLIFETIME_CONDITION(
		ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);

	// Register the player's current health to be replicated by the server
	DOREPLIFETIME(ABlasterCharacter, CurrentHealth);

	// Register the whether or not to disable gameplay for the current player to
	// be replicated by the server
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	HideCameraIfCharacterIsClose();
	PollInit();
}

void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	// Disable rotation and turning functionality if we're in the cooldown state
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;

		// For simulated proxies, make sure to call OnRep_ReplicatedMovement
		// after an interval if the player hasn't moved, since the function is
		// normally only called when a player moves
		const float MaxMovementReplicationInterval = 0.25;

		if (TimeSinceLastMovementReplication > MaxMovementReplicationInterval)
		{
			OnRep_ReplicatedMovement();
		}

		// Calculate the pitch every frame still
		CalculateAO_Pitch();
	}
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	// We're calling this here, because we only want to calculate the rotation
	// for every net update, instead of every frame
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.0f;
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage,
	const UDamageType* DamageType, AController* InstigatorController,
	AActor* DamageCauser)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
	PlayHitReactMontage();
	UpdateHUDHealth();

	if (CurrentHealth == 0.0f)
	{
		ABlasterGameMode* BlasterGameMode =
			GetWorld()->GetAuthGameMode<ABlasterGameMode>();

		if (BlasterGameMode)
		{
			// Only cast to the player controller if we don't have it already
			// yet for some reason
			if (!BlasterPlayerController)
			{
				BlasterPlayerController =
					Cast<ABlasterPlayerController>(Controller);
			}

			// Cast the attacker's controller to our custom player controller
			// class as well
			ABlasterPlayerController* AttackerController =
				Cast<ABlasterPlayerController>(InstigatorController);

			// We'll check to make sure all the controllers are valid in the
			// game mode
			BlasterGameMode->PlayerEliminated(
				this, BlasterPlayerController, AttackerController);
		}
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	if (!BlasterPlayerController)
	{
		BlasterPlayerController = Cast<ABlasterPlayerController>(Controller);
	}

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(CurrentHealth, MaxHealth);
	}
}

void ABlasterCharacter::Eliminated()
{
	// Drop the weapon the player's holding if any
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}

	// Call the multicast RPC to replicate the player's elimination
	MulticastEliminated();

	// Start the respawn timer
	GetWorldTimerManager().SetTimer(
		ElimTimer, this, &ABlasterCharacter::ElimTimerFinished, ElimDelay);
}

void ABlasterCharacter::MulticastEliminated_Implementation()
{
	// Set the player's ammo to zero
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}

	bIsEliminated = true;
	PlayElimMontage();

	// Create a dynamic material instance of the dissolve material to be used to
	// apply the dissolve effect to the character
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance =
			UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);

		// Set the starting values for the dynamic material instance
		DynamicDissolveMaterialInstance->SetScalarParameterValue(
			TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(
			TEXT("Glow"), 200.0f);
	}

	// Start the dissolve effect
	StartDissolve();

	// Disable movement for the character
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	// Disable gameplay related actions
	bDisableGameplay = true;
	
	// Stop firing when eliminated
	if (Combat)
	{
		Combat->bFireButtonPressed = false;
	}

	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Spawn elimination bot
	if (ElimBotEffect)
	{
		// Spawn the bot 1.9 meters above the player's head
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y,
			GetActorLocation().Z + 190.0f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(), ElimBotEffect, ElimBotSpawnPoint, GetActorRotation());
	}

	// Play the elim bot sound
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this, ElimBotSound, GetActorLocation());
	}
}

void ABlasterCharacter::ElimTimerFinished()
{
	ABlasterGameMode* BlasterGameMode =
		GetWorld()->GetAuthGameMode<ABlasterGameMode>();

	// Respawn the player upon timer finish
	if (BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	// Destroy the elim bot.
	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	ABlasterGameMode* BlasterGameMode =
		Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));

	bool bMatchEnded = BlasterGameMode &&
					   BlasterGameMode->GetMatchState() != MatchState::InProgress;

	// Destroy the equipped weapon on the character if it doesn't get dropped
	// when the level gets reset
	if (Combat && Combat->EquippedWeapon && bMatchEnded)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void ABlasterCharacter::PollInit()
{
	// If the player state isn't initialized, initialize it
	if (!BlasterPlayerState)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();

		// Set the HUD widgets to their starting values once the player state
		// has been initialized
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.0f);
			BlasterPlayerState->AddToElimCount(0);
		}
	}

	/** TODO: Initialize the HUD weapon ammo to zero */
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize the healthbar on the player's HUD
	BlasterPlayerController = Cast<ABlasterPlayerController>(Controller);
	UpdateHUDHealth();

	// We only want damage to be handled by the server, so we're binding the
	// callback ReceiveDamage only if we're the server
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}

	// Hide the player's grenade on startup
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(
	UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(
		"Jump", IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction(
		"Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction(
		"Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction(
		"Reload", IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction(
		"Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction(
		"Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction(
		"Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction(
		"Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction(
		"ThrowGrenade", IE_Pressed, this, &ABlasterCharacter::GrenadeButtonPressed);

	PlayerInputComponent->BindAxis(
		"MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis(
		"MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);
	PlayerInputComponent->BindAxis(
		"LookRight", this, &ABlasterCharacter::LookRight);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (!Combat || !Combat->EquippedWeapon)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);

		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (!Combat || !Combat->EquippedWeapon)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);

		FName SectionName;

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
			case EWeaponType::EWT_AssaultRifle:
			{
				SectionName = FName("Rifle");
				break;
			}
			case EWeaponType::EWT_RocketLauncher:
			{
				SectionName = FName("RocketLauncher");
				break;
			}
			case EWeaponType::EWT_Pistol:
			{
				SectionName = FName("Pistol");
				break;
			}
			// Just using the rifle montage here
			case EWeaponType::EWT_SubmachineGun:
			{
				SectionName = FName("Rifle");
				break;
			}
			case EWeaponType::EWT_Shotgun:
			{
				SectionName = FName("Shotgun");
				// Speed this one up a bit
				AnimInstance->Montage_SetPlayRate(ReloadMontage, 1.2f);
				break;
			}
			case EWeaponType::EWT_SniperRifle:
			{
				SectionName = FName("SniperRifle");
				break;
			}
			// Just using the pistol montage here
			case EWeaponType::EWT_GrenadeLauncher:
			{
				SectionName = FName("Pistol");
				break;
			}
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (!Combat || !Combat->EquippedWeapon)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);

		FName SectionName = FName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::MoveForward(float Value)
{
	// Disable this action if true
	if (bDisableGameplay)
	{
		return;
	}

	if (Controller && Value != 0.0f)
	{
		const FRotator YawRotation(
			0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		// Get the direction of YawRotation
		const FVector Direction(
			FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));

		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	// Disable this action if true
	if (bDisableGameplay)
	{
		return;
	}

	if (Controller && Value != 0.0f)
	{
		const FRotator YawRotation(
			0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		// Get the direction of YawRotation
		const FVector Direction(
			FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));

		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookRight(float Value)
{
	AddControllerPitchInput(Value);
}

// NOTE: This is for the server only. Clients will equip weapons through sending
// a message to the server through ServerEquipButtonPressed
void ABlasterCharacter::EquipButtonPressed()
{
	// Disable this action if true
	if (bDisableGameplay)
	{
		return;
	}

	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

// Equips weapons for server clients. This will send a message to the server so
// that it can equip the weapon for them
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	// Disable this action if true
	if (bDisableGameplay)
	{
		return;
	}

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	// Disable this action if true
	if (bDisableGameplay)
	{
		return;
	}

	if (Combat)
	{
		Combat->Reload();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	// Disable this action if true
	if (bDisableGameplay)
	{
		return;
	}

	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	// Disable this action if true
	if (bDisableGameplay)
	{
		return;
	}

	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	// Disable this action if true
	if (bDisableGameplay)
	{
		return;
	}

	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	// Disable this action if true
	if (bDisableGameplay)
	{
		return;
	}

	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::GrenadeButtonPressed()
{
	if (Combat)
	{
		Combat->ThrowGrenade();
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	// If we're not holding a weapon
	if (Combat && Combat->EquippedWeapon == nullptr)
	{
		return;
	}

	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	// If we're standing still and not jumping
	if (Speed == 0.f && !bIsInAir)
	{
		bShouldRotateRootBone = true;

		FRotator CurrentAimRotation =
			FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = UKismetMathLibrary::NormalizedDeltaRotator(
			CurrentAimRotation, StartingAimRotation)
					 .Yaw;

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}

		bUseControllerRotationYaw = true;

		// Turn in place based on the direction we're aiming
		TurnInPlace(DeltaTime);
	}

	// If we're running or jumping
	if (Speed > 0.0f || bIsInAir)
	{
		bShouldRotateRootBone = false;

		StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		AO_Yaw = 0.0f;

		bUseControllerRotationYaw = true;

		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();

	// if (GEngine)
	//{
	//	GEngine->AddOnScreenDebugMessage(
	//		0,
	//		15.0f,
	//		FColor::Green,
	//		FString::Printf(TEXT("AO_Yaw: %f\nAO_Pitch: %f"), AO_Yaw, AO_Pitch)
	//	);
	// }
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	// We can set the pitch regardless if we're running or jumping
	AO_Pitch = GetBaseAimRotation().Pitch;

	// Do the unpacking of the compressed pitch data if it's been sent to us
	// from the server
	if (AO_Pitch > 90.0f && !IsLocallyControlled())
	{
		FVector2D InRange(270.0f, 360.0f);
		FVector2D OutRange(-90.0f, 0.0f);
		AO_Pitch =
			FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (!Combat || !Combat->EquippedWeapon)
	{
		return;
	}

	// Don't rotate root bones for simulated proxies as they don't update every
	// frame, which causes the rotation to look jittery
	bShouldRotateRootBone = false;

	// If we're running, set turning to none and return early so we can end the
	// turning animation
	float Speed = CalculateSpeed();

	if (Speed > 0.0f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	// Calculate the difference in yaw rotation between the last and current
	// frame
	ProxyRotationSinceLastFrame = ProxyRotationCurrentFrame;
	ProxyRotationCurrentFrame = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(
		ProxyRotationCurrentFrame, ProxyRotationSinceLastFrame)
				   .Yaw;

	// UE_LOG(LogTemp, Warning, TEXT("ProxyYaw: %f"), ProxyYaw);

	// Turn in place if we've rotated above the required amount
	if (FMath::Abs(ProxyYaw) > TurnInPlaceThreshold)
	{
		// Turning right
		if (ProxyYaw > TurnInPlaceThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		// Turning left
		else if (ProxyYaw < -TurnInPlaceThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}

		return;
	}

	// If we're not turning
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::Jump()
{
	// Disable this action if true
	if (bDisableGameplay)
	{
		return;
	}

	// Just stand back up if we're crouching
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.0f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.0f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	// Interpolate if we are turning
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.0f, DeltaTime, 4.0f);
		AO_Yaw = InterpAO_Yaw;

		// Stop turning and reset our starting aim rotation if we're below the
		// minimum bound
		float MinimumTurnAngle = 15.0f;

		if (FMath::Abs(AO_Yaw) < MinimumTurnAngle)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation =
				FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		}
	}
}

void ABlasterCharacter::HideCameraIfCharacterIsClose()
{
	// Local players only
	if (!IsLocallyControlled())
	{
		return;
	}

	float CharacterDistanceFromCamera =
		(FollowCamera->GetComponentLocation() - GetActorLocation()).Size();

	if (CharacterDistanceFromCamera < HideCharacterDistance)
	{
		GetMesh()->SetVisibility(false);

		// Disable visibility on the weapon mesh only for the owner
		if (Combat && Combat->EquippedWeapon &&
			Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);

		// Disable visibility on the weapon mesh only for the owner
		if (Combat && Combat->EquippedWeapon &&
			Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	float Speed = Velocity.Size();

	return Speed;
}

void ABlasterCharacter::OnRep_Health()
{
	PlayHitReactMontage();
	UpdateHUDHealth();
}

void ABlasterCharacter::StartDissolve()
{
	// Bind the callback function for the dissolve track to call every frame
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);

	// Add the curve for the dissolve timeline and start playing it
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(
			TEXT("Dissolve"), DissolveValue);
	}
}

// NOTE: This function is only called on the server. We'll use server
// replication with OnRep_OverlappingWeapon to relay the weapon info to the
// clients.
void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	// Disable the pickup widget if the overlap event triggers and we were
	// overlapping a weapon.
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;

	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

// Variable replication implementation
void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	// Check to see if a weapon is being overlapped. If the weapon is null this
	// if check will fail and we will execute the bottom if check
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}

	// In order to check if the weapon has stopped being overlapped, we need to
	// check if the server contained a pointer to a valid weapon class before it
	// was changed to null
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bIsAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (!Combat)
	{
		return nullptr;
	}

	return Combat->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (!Combat)
	{
		return FVector();
	}

	return Combat->HitTarget;
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (!Combat)
	{
		return ECombatState::ECS_MAX;
	}

	return Combat->CombatState;
}
