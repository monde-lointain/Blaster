// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "WeaponTypes.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Weapon.generated.h"

class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class ABulletShell;
class UTexture2D;
class ABlasterCharacter;
class ABlasterPlayerController;
class USoundCue;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values */
	AWeapon();

	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	/** Sets the properties for lifetime server replication */
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Replication notify for owner switching */
	virtual void OnRep_Owner() override;

	/** Updates the weapon ammo counter on the owning player's HUD */
	void SetAmmoCountOnOwnerHUD();

	/** Turns the widget displayed when overlapping a weapon on and off */
	void ShowPickupWidget(bool bShowWidget);

	/** Handles elements of firing assigned to the weapon */
	virtual void Fire(const FVector& HitTarget);

	/** Handles dropping the weapon */
	void Dropped();

	/** Handles putting ammo into the weapon when reloading */
	void AddAmmo(int32 AmountToAdd);

	/** Textures for the weapon crosshairs */
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;

	/** Zoomed FOV while aiming */
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.0f;

	/** Interpolation speed for the aim zoom */
	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.0f;

	/** Interval between shots for automatic weapons */
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.15f;

	/** Indicates whether a weapon is automatic or not */
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bIsAutomatic = true;

	/**
	 * Sound that plays when equipping a weapon. Changes depending on the
	 * weapon type
	 */
	UPROPERTY(EditAnywhere)
	USoundCue* EquipSound;

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/**
	 * Handles events for the server when a weapons sphere component is
	 * overlapped.
	 */
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * Handles events for the server when a weapons sphere component has stopped
	 * being overlapped
	 */
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex);

private:
	/** The owning character of the weapon */
	UPROPERTY()
	ABlasterCharacter* BlasterOwnerCharacter;

	/** The owning controller associated with the character owning the weapon */
	UPROPERTY()
	ABlasterPlayerController* BlasterOwnerController;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	/** Used to trigger events on the server when overlapped. */
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USphereComponent* AreaSphere;

	/** The current state of the weapon. */
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere,
		Category = "Weapon Properties")
	EWeaponState WeaponState;

	/** Handles changes in the weapon state for clients */
	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UWidgetComponent* PickupWidget;

	/** Class used for the bullet shell casing the weapon spawns */
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABulletShell> BulletShellClass;

	/** Enables and disables physics for the weapon mesh */
	void SetWeaponPhysicsEnabled(bool bPhysicsEnabled);

	/** Current ammo for the weapon */
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	/** The max ammo the weapon can hold */
	UPROPERTY(EditAnywhere)
	int32 MaxAmmo;

	/**
	 * Expends the round fired and updates the current ammo count for the
	 * weapon
	 */
	void SpendRound();

	/** Replication notify for ammo count */
	UFUNCTION()
	void OnRep_Ammo();

	/** The weapon type */
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

public:
	/** Handles changes in the weapon state for the server */
	void SetWeaponState(EWeaponState State);

	FORCEINLINE USphereComponent* GetAreaSphere() const
	{
		return AreaSphere;
	}

	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const
	{
		return WeaponMesh;
	}

	UPROPERTY(EditAnywhere, Category = WeaponProperties)
	UAnimationAsset* FireAnimation;

	FORCEINLINE float GetZoomedFOV() const
	{
		return ZoomedFOV;
	}

	FORCEINLINE float GetZoomInterpSpeed() const
	{
		return ZoomInterpSpeed;
	}

	FORCEINLINE EWeaponType GetWeaponType() const
	{
		return WeaponType;
	}

	FORCEINLINE int32 GetCurrentAmmo() const
	{
		return Ammo;
	}

	FORCEINLINE int32 GetMaxAmmo() const
	{
		return MaxAmmo;
	}

	/** Returns true if Ammo == 0 */
	bool IsEmpty();

	/** Returns true if Ammo == MaxAmmo */
	bool IsFull();
};