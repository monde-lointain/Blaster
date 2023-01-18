// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	BaseWalkSpeed = 1000.0f;
	AimWalkSpeed = 750.0f;
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
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

	if (Character)
	{
		Character->PlayFireMontage(bIsAiming);
	}
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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
