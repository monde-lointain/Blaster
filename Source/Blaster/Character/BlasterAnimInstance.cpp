// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterAnimInstance.h"

#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Weapon/Weapon.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!BlasterCharacter)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}

	if (!BlasterCharacter)
	{
		return;
	}

	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.0f;
	Speed = Velocity.Size();

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	bIsCrouched = BlasterCharacter->bIsCrouched;
	bIsAiming = BlasterCharacter->IsAiming();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();

	float AccelerationMagnitude = BlasterCharacter->GetCharacterMovement()
									  ->GetCurrentAcceleration()
									  .Size();
	bIsAccelerating = AccelerationMagnitude > 0.0f ? true : false;

	// Calculate the offset yaw for strafing
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Velocity);
	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(
		MovementRotation, AimRotation);
	// Interpolate the yaw offset to prevent jerkiness between strafing left and
	// right
	DeltaRotation = FMath::RInterpTo(DeltaRotation, Delta, DeltaTime, 5.0f);
	YawOffset = DeltaRotation.Yaw;

	// The lean is the difference in yaw rotation between the character and it's
	// yaw rotation from the previous frame
	CharacterRotationLastFrame = CharacterRotationCurrentFrame;
	CharacterRotationCurrentFrame = BlasterCharacter->GetActorRotation();
	const float DeltaYaw = UKismetMathLibrary::NormalizedDeltaRotator(
		CharacterRotationCurrentFrame, CharacterRotationLastFrame)
							   .Yaw;

	// Interpolate the lean to prevent any jerkiness
	const float Target = DeltaYaw / DeltaTime;
	const float Interpolated = FMath::FInterpTo(Lean, Target, DeltaTime, 6.0f);

	// Clamp to prevent super fast mouse movements from causing the lean to go
	// outside the bounds
	Lean = FMath::Clamp(Interpolated, -90.0f, 90.0f);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			1,
			15.0f,
			FColor::Green,
			FString::Printf(TEXT("Lean: %f"), Lean)
		);
	}

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	// Set up FABRIK
	if (bWeaponEquipped &&
		EquippedWeapon &&
		EquippedWeapon->GetWeaponMesh() &&
		BlasterCharacter->GetMesh())
	{
		// Get the left hand transform in world space, then transform it to bone
		// space. 
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(
			FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);

		FVector OutPosition;
		FRotator OutRotation;

		// Use the right hand bone as the reference bone
		BlasterCharacter->GetMesh()->TransformToBoneSpace(
			FName("hand_r"),
			LeftHandTransform.GetLocation(), 
			FRotator::ZeroRotator, 
			OutPosition,
			OutRotation
		);

		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
	}
}
