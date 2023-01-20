// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "BlasterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;
class UAnimMontage;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter,
									  public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	/** Sets default values */
	ABlasterCharacter();

	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	/** Called when the game starts or when spawned */
	virtual void SetupPlayerInputComponent(
		class UInputComponent* PlayerInputComponent) override;

	/** Sets the properties for lifetime server replication */
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Called after initializing all components. In this case we use it to set
	 * some variables of those components from within this class
	 */
	virtual void PostInitializeComponents() override;

	/** Handles playing montages for a player firing weapons and getting hit */
	void PlayFireMontage(bool bAiming);
	void PlayHitReactMontage();

	/** Server RPC for replicating hits */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHit();

	/** Overridden to handle movement replication for simulated proxies */
	virtual void OnRep_ReplicatedMovement() override;

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void LookRight(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();

	/**
	 * Calculates the aim offset between the direction the player is facing and
	 * the direction they're pointing their weapon in
	 */
	void AimOffset(float DeltaTime);

	void CalculateAO_Pitch();

	/** Handles the yaw rotation for non-locally controlled characters */
	void SimProxiesTurn();

	virtual void Jump() override;

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;

	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	UCombatComponent* Combat;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	/** Used for calculating the aim offset */
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	/** Indicates the player's current turning in place state */
	ETurningInPlace TurningInPlace;

	void TurnInPlace(float DeltaTime);

	/** Animation montage for firing weapons */
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* FireWeaponMontage;

	/** Animation montage for getting hit */
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	void HideCameraIfCharacterIsClose();

	/** Minimum distance from the player to the camera before hiding them */
	UPROPERTY(EditAnywhere)
	float HideCharacterDistance = 200.0f;

	/** Indicates whether a character should rotate its root bone or not */
	bool bShouldRotateRootBone;

	/** Rotation angle for simulated proxies to turn in place after */
	float TurnInPlaceThreshold = 0.5f;

	/** Amount a simulated proxy has rotated since last frame */
	FRotator ProxyRotationSinceLastFrame;

	/** Amount a simulated proxy has rotated this frame */
	FRotator ProxyRotationCurrentFrame;

	/**
	 * Difference in yaw rotation between the last and current frame for a
	 * simulated proxy
	 */
	float ProxyYaw;

	float TimeSinceLastMovementReplication;

	float CalculateSpeed();

public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();

	FORCEINLINE float GetAO_Yaw() const
	{
		return AO_Yaw;
	}

	FORCEINLINE float GetAO_Pitch() const
	{
		return AO_Pitch;
	}

	AWeapon* GetEquippedWeapon();

	FORCEINLINE ETurningInPlace GetTurningInPlace() const
	{
		return TurningInPlace;
	}

	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent* GetFollowCamera() const
	{
		return FollowCamera;
	}

	FORCEINLINE bool ShouldRotateRootBone() const
	{
		return bShouldRotateRootBone;
	}
};
