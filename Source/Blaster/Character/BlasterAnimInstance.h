// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimInstance.h"
#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"

#include "BlasterAnimInstance.generated.h"

class ABlasterCharacter;
class AWeapon;

/**
 *
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	UPROPERTY(BlueprintReadOnly, Category = Character,
		meta = (AllowPrivateAccess = true))
	ABlasterCharacter* BlasterCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Character,
		meta = (AllowPrivateAccess = true))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = Character,
		meta = (AllowPrivateAccess = true))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Character,
		meta = (AllowPrivateAccess = true))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = Character,
		meta = (AllowPrivateAccess = true))
	bool bWeaponEquipped;

	AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = Character,
		meta = (AllowPrivateAccess = true))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = Character,
		meta = (AllowPrivateAccess = true))
	bool bIsAiming;

	/** Drives the x axis of the leaning and strafing blendspace */
	UPROPERTY(BlueprintReadOnly, Category = Character,
		meta = (AllowPrivateAccess = true))
	float YawOffset;

	/** Drives the y axis of the leaning and strafing blendspace */
	UPROPERTY(BlueprintReadOnly, Category = Character,
		meta = (AllowPrivateAccess = true))
	float Lean;

	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotationCurrentFrame;

	/** Used to interpolate the offset yaw for strafing */
	FRotator DeltaRotation;

	/** Drives the yaw of the aim offset */
	UPROPERTY(BlueprintReadOnly, Category = Character,
		meta = (AllowPrivateAccess = true))
	float AO_Yaw;

	/** Drives the pitch of the aim offset */
	UPROPERTY(BlueprintReadOnly, Category = Character,
		meta = (AllowPrivateAccess = true))
	float AO_Pitch;

	/** Used for placing the left hand on the weapon via FABRIK */
	UPROPERTY(BlueprintReadOnly, Category = Character,
		meta = (AllowPrivateAccess = true))
	FTransform LeftHandTransform;

	/** Indicates the player's current turning in place state */
	UPROPERTY(BlueprintReadOnly, Category = Character,
		meta = (AllowPrivateAccess = true))
	ETurningInPlace TurningInPlace;
};
