// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blaster/HUD/BlasterHUD.h"
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

	void EquipWeapon(AWeapon* WeaponToEquip);

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

private:
	ABlasterCharacter* Character;

	ABlasterPlayerController* Controller;

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

	/** Indicates whether the fire buton has been pressed or not. */
	bool bFireButtonPressed;

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

	/** Indicates whether the player can fire or not */
	bool bCanFire = true;

	/** Handles the timer for intervals between firing for automatic weapons */
	FTimerHandle FireTimer;

	/** Sets the fire timer for a player */
	void StartFireTimer();

	/** Callback for when the fire timer has ended */
	void FireTimerFinished();
};
