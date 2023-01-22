// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"

#include "BlasterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;
class UAnimMontage;
class ABlasterPlayerController;
class AController;

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

	/** These handle playing various animation montages for the player */
	void PlayFireMontage(bool bAiming);
	void PlayHitReactMontage();
	void PlayElimMontage();

	/** Overridden to handle movement replication for simulated proxies */
	virtual void OnRep_ReplicatedMovement() override;

	/** Handles damage taken events for the player */
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage,
		const UDamageType* DamageType, AController* InstigatorController,
		AActor* DamageCauser);

	/** Updates the HUD health widget of the player after taking damage */
	void UpdateHUDHealth();

	/** Server call for handling player elimination */
	void Eliminated();

	/** Multicast RPC for handling player elimination */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminated();

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

	/** Animation montage for getting eliminated */
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

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

	/** The player's current health */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Health, Category = PlayerStats)
	float CurrentHealth = 100.0f;

	/** The player's maximum health */
	UPROPERTY(EditAnywhere, Category = PlayerStats)
	float MaxHealth = 100.0f;

	/** Indicates whether the player has been eliminated or not */
	bool bIsEliminated = false;

	/** Replication notify for health */
	UFUNCTION()
	void OnRep_Health();

	/**
	 * Represents the controller the player is currently using. Used for
	 * accessing the HUD in the character class
	 */
	ABlasterPlayerController* BlasterPlayerController;

	/** Handle to the player respawn timer */
	FTimerHandle ElimTimer;

	/** Respawn timer length */
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.0f;

	/** Callback function for the player eliminated timer */
	void ElimTimerFinished();

	/**
	 * Material instance used for the dissolve effect on the character upon
	 * elimination. We'll set this in our character blueprint to determine what
	 * material gets applied to our character upon elimination
	 */
	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* DissolveMaterialInstance;

	/**
	 * Dynamic instance of our dissolve material that we can change at runtime
	 * to apply the dissolve effect to the character
	 */
	UPROPERTY(VisibleAnywhere, Category = Elimination)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	/** Dissolve effect timeline */
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	/** Track for the dissolve effect timeline */
	FOnTimelineFloat DissolveTrack;

	/** Curve for the dissolve effect timeline */
	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	/**
	 * Starts the dissolve timeline and binds the dissolve callback funtion for
	 * the dissolve track
	 */
	void StartDissolve();

	/** Updates the dissolve timeline each frame */
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

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

	FORCEINLINE bool IsEliminated() const
	{
		return bIsEliminated;
	}
};
