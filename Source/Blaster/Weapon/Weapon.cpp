// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "BulletShell.h"
#include "Engine/SkeletalMeshSocket.h"

// Sets default values
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Initialize the weapon mesh
	WeaponMesh =
		CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	// Set the collision for the mesh
	WeaponMesh->SetCollisionResponseToAllChannels(
		ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Initialize the pickup sphere for the weapon
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);

	// Set collision for the pickup sphere
	AreaSphere->SetCollisionResponseToAllChannels(
		ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Initialize the hover text that appears when overlapping the pickup sphere
	PickupWidget =
		CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	// Enable collision and add the special overlap events ONLY if we're the
	// server. We can handle the overlap events for clients with server
	// replication
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(
			ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(
			this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(
			this, &AWeapon::OnSphereEndOverlap);
	}
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Register the weapon state to be replicated by the server
	DOREPLIFETIME(AWeapon, WeaponState);

	// Register the ammo count in the weapon to be replicated by the server
	DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);

	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);

	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetAmmoCountOnOwnerHUD()
{
	// Check to make sure the owning character and controller are valid, then
	// update the HUD with the weapon's current ammo count
	if (!BlasterOwnerCharacter)
	{
		BlasterOwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
	}
	if (BlasterOwnerCharacter)
	{
		if (!BlasterOwnerController)
		{
			BlasterOwnerController = Cast<ABlasterPlayerController>(
				BlasterOwnerCharacter->Controller);
		}
		if (BlasterOwnerController)
		{
			BlasterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::SpendRound()
{
	// Make sure the ammo count can never go below zero or above the max ammo
	// for the weapon
	Ammo = FMath::Clamp(Ammo - 1, 0, MaxAmmo);
	SetAmmoCountOnOwnerHUD();
}

void AWeapon::OnRep_Ammo()
{
	SetAmmoCountOnOwnerHUD();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	// If the weapon's been dropped or otherwise doesn't have an owner
	if (!Owner)
	{
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerController = nullptr;
	}
	else
	{
		// Update the ammo count on the new owner's HUD
		SetAmmoCountOnOwnerHUD();
	}
}

// Sets the weapon state for the server. The changed weapon state will be
// replicated to clients through OnRep_WeaponState
void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;

	switch (WeaponState)
	{
		case EWeaponState::EWS_Equipped:
		{
			ShowPickupWidget(false);

			// We only need to disable collision for the area sphere on the
			// server because overlap events only occur on it
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Disable physics on the weapon when the player equips it
			SetWeaponPhysicsEnabled(false);
			break;
		}
		case EWeaponState::EWS_Dropped:
		{
			// Make sure we only reenable collision on the server
			if (HasAuthority())
			{
				AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			}

			// Enable physics when the weapon gets dropped so we can see the
			// weapon dropping and bouncing on the ground
			SetWeaponPhysicsEnabled(true);
			break;
		}
	}
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}

// Handles the replication of the weapon's state from the server to the clients
void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
		case EWeaponState::EWS_Equipped:
		{
			ShowPickupWidget(false);
			// Disable physics on the weapon when the player equips it
			SetWeaponPhysicsEnabled(false);
			break;
		}
		case EWeaponState::EWS_Dropped:
		{
			// Enable physics when the weapon gets dropped so we can see the
			// weapon dropping and bouncing on the ground
			SetWeaponPhysicsEnabled(true);
			break;
		}
	}
}

void AWeapon::SetWeaponPhysicsEnabled(bool bPhysicsEnabled)
{
	if (bPhysicsEnabled)
	{
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else
	{
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	if (BulletShellClass)
	{
		// Get the socket to spawn the casing at
		const USkeletalMeshSocket* CasingSocket =
			WeaponMesh->GetSocketByName(FName("AmmoEject"));

		if (CasingSocket)
		{
			// Get the location to spawn the casing at from the socket
			FTransform SocketTransform =
				CasingSocket->GetSocketTransform(WeaponMesh);

			// Spawn the projectile at the target location
			UWorld* World = GetWorld();

			if (World)
			{
				World->SpawnActor<ABulletShell>(
					BulletShellClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
					);
			}
		}
	}

	// Reduce the ammo count by one and update the player HUD
	SpendRound();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	// Detach the weapon from the character
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);

	// Clear the weapon's owner
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

void AWeapon::AddAmmo(int32 AmountToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmountToAdd, 0, MaxAmmo);
	SetAmmoCountOnOwnerHUD();
}
