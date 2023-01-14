// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterCharacter.h"

#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/BlasterComponents/CombatComponent.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to
	// improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom =
		CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera =
		CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget =
		CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);
}

void ABlasterCharacter::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Register the weapon class to be replicated by the server
	DOREPLIFETIME_CONDITION(
		ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(
	UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(
		"Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction(
		"Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);

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

void ABlasterCharacter::MoveForward(float Value)
{
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

// NOTE: This function is only called for the server. We'll use server
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
