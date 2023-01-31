// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "CombatComponent.generated.h"

class AWeapon;
class ABlasterCharacter;
class ABlasterPlayerController;
class ABlasterHUD;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	friend class ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Sets the properties for lifetime server replication */
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Handles equipping weapons */
	void EquipWeapon(AWeapon* WeaponToEquip);

	void ReloadEmptyWeapon();

	void PlayEquipWeaponSound();

	/** Updates the ammo counts on the ammo map and the HUD */
	void UpdateCarriedAmmo();

	void DropEquippedWeapon();

	/** Clientside reload function */
	void Reload();

	/** Ends the reloading process */
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	/** Updates the ammo count on reload */
	void UpdateAmmoCounts();

	/** Updates the ammo count for the shotgun on reload */
	void UpdateShotgunAmmoCounts();

	/** Indicates whether the fire buton has been pressed or not. */
	bool bFireButtonPressed;

	/** Handles reloading shotgun shells. */
	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	/**
	 * Goes to the end of the shotgun reload montage. Called after breaking the
	 * reload cycle in the shotgun
	 */
	void JumpToShotgunEnd();

	/** Handles throwing grenades */
	void ThrowGrenade();

	/** Server RPC for throwing grenades */
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	/**
	 * Indicates when the player has finished throwing a grenade so we can set
	 * their combat state back to unoccupied
	 */
	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

protected:
	virtual void BeginPlay() override;

	/**
	 * Handles aiming for the server. The server will have the aiming flag set
	 * in SetAiming, then replicate it down to the clients with ServerSetAiming.
	 */
	void SetAiming(bool bAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming);

	/**
	 * Server variable replication for the movement parameters on equipping a
	 * weapon
	 */
	UFUNCTION()
	void OnRep_EquippedWeapon();

	/** For attaching actors to the character's hands */
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);

	/** Called whenever a player presses the fire button. */
	void FireButtonPressed(bool bPressed);

	void Fire();

	/**
	 * Server RPC for firing weapons. Called by clients and executed on the
	 * server
	 */
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	/**
	 * Multicast RPC for firing weapons. Called by the server and replicated
	 * down to all clients
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	/** Performs a line trace starting from the center of the screen */
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	/** Sets the textures for the HUD crosshairs */
	void SetHUDCrosshairs(float DeltaTime);

	/** Server RPC for reloading */
	UFUNCTION(Server, Reliable)
	void ServerReload();

	/**
	 * Handles reload functionality that should happen on both server and
	 * clients
	 */
	void HandleReload();

	/** Gets the amount of ammo to reload when reloading a weapon */
	int32 CalculateReloadAmmo();

private:
	/** The character using this component */
	UPROPERTY()
	ABlasterCharacter* Character;

	/** The player controller using this component */
	UPROPERTY()
	ABlasterPlayerController* Controller;

	/** The HUD used by the player using this component */
	UPROPERTY()
	ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bIsAiming;

	/** Walking speed while not aiming */
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	/** 
	 * Walking speed while aiming. Has to be replicated from server to clients
	 * within ServerSetAiming
	 * Note that we need to set these here, because the base character movement
	 * functionality only provides movement speeds for walking and crouching.
	 */
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	/**
	 * Factor for how calculating how much the player's HUD crosshairs should be
	 * spread based on their velocity
	 */
	float CrosshairVelocityFactor;

	/**
	 * Factor for how calculating how much the player's HUD crosshairs should be
	 * spread when in the air
	 */
	float CrosshairInAirFactor;

	/**
	 * Factor for how calculating how much the player's HUD crosshairs should
	 * contract when aiming
	 */
	float CrosshairAimFactor;

	/**
	 * Factor for how calculating how much the player's HUD crosshairs should
	 * spread when shooting
	 */
	float CrosshairShootFactor;

	/** Impact point calculated from the line trace in TraceUnderCrosshairs */
	FVector HitTarget;

	/** Parameters for the player HUD */
	FHUDPackage HUDPackage;

	/**
	 * Field of view when not aiming; set to the camera's base FOV in BeginPlay
	 */
	float DefaultFOV;

	/** Field of view when zoomed */
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.0f;

	float CurrentFOV;

	/** Interpolation speed for the aim zoom */
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.0f;

	/** Interpolates FOV when aiming */
	void InterpFOV(float DeltaTime);

	/** Sound that plays on sniper rifle zoom in */
	UPROPERTY(EditAnywhere, Category = SniperRifleSounds)
	USoundBase* ZoomInSniperRifle;

	/** Sound that plays on sniper rifle zoom out */
	UPROPERTY(EditAnywhere, Category = SniperRifleSounds)
	USoundBase* ZoomOutSniperRifle;

	/** Indicates whether the player can fire or not */
	bool bCanFire = true;

	/** Handles the timer for intervals between firing for automatic weapons */
	FTimerHandle FireTimer;

	/** Sets the fire timer for a player */
	void StartFireTimer();

	/** Callback for when the fire timer has ended */
	void FireTimerFinished();

	bool CanFire();

	/**
	 * Indicates the ammo carried on the player for the currently equipped
	 * weapon
	 */
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	/** Replication notify for carried ammo */
	UFUNCTION()
	void OnRep_CarriedAmmo();

	/** Map for the ammo counts being carried on the player for each weapon type */
	TMap<EWeaponType, int32> CarriedAmmoMap;

	/** Starting ammo for assault rifles  */
	UPROPERTY(EditAnywhere, Category = StartingAmmo)
	int AssaultRifleStartAmmo = 30;

	/** Starting ammo for rocket launchers  */
	UPROPERTY(EditAnywhere, Category = StartingAmmo)
	int RocketLauncherStartAmmo = 0;

	/** Starting ammo for pistols  */
	UPROPERTY(EditAnywhere, Category = StartingAmmo)
	int PistolStartAmmo = 0;

	/** Starting ammo for submachine guns  */
	UPROPERTY(EditAnywhere, Category = StartingAmmo)
	int SubmachineGunStartAmmo = 0;

	/** Starting ammo for shotguns  */
	UPROPERTY(EditAnywhere, Category = StartingAmmo)
	int ShotgunStartAmmo = 0;

	/** Starting ammo for sniper rifles  */
	UPROPERTY(EditAnywhere, Category = StartingAmmo)
	int SniperRifleStartAmmo = 0;

	/** Starting ammo for sniper rifles  */
	UPROPERTY(EditAnywhere, Category = StartingAmmo)
	int GrenadeLauncherStartAmmo = 0;

	/**
	 * Sets the carried ammo amounts for each weapon type to their starting
	 * values
	 */
	void InitializeCarriedAmmo();

	/** The current state of the character in combat */
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	/** Replication notify for combat state */
	UFUNCTION()
	void OnRep_CombatState();
};
