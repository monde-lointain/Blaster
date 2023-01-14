// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
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
}
