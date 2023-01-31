// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterAnimInstance.h"

#include "BlasterCharacter.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/Weapon/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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
	bShouldRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	bIsEliminated = BlasterCharacter->IsEliminated();

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

	//if (GEngine)
	//{
	//	GEngine->AddOnScreenDebugMessage(
	//		1,
	//		15.0f,
	//		FColor::Green,
	//		FString::Printf(TEXT("Lean: %f"), Lean)
	//	);
	//}

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

		// Apply a correction rotation to the right hand bone for the local
		// player so they can see their gun pointed in the right direction
		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;

			FVector RightHandTransformLocation =
				EquippedWeapon->GetWeaponMesh()
				->GetSocketTransform(FName("Hand_R"),
					ERelativeTransformSpace::RTS_World)
				.GetLocation();

			// Since the x axis of the right hand points in the opposite direction
			// of the gun, we will get the rotation in the opposite direction by
			// using a vector going backwards in the opposite direction of the hit
			// target
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(
				RightHandTransformLocation,
				RightHandTransformLocation + (RightHandTransformLocation
					                          - BlasterCharacter->GetHitTarget())
			);

			// Smooth out the rotation
			RightHandRotation = FMath::RInterpTo(
				RightHandRotation, LookAtRotation, DeltaTime, 30.f);

			//UE_LOG(LogTemp, Warning, TEXT("RightHandRotation: %s"),
			//	*RightHandRotation.ToString());
		}

		///** Debug lines */

		//FTransform MuzzleTipTransform =
		//	EquippedWeapon->GetWeaponMesh()->GetSocketTransform(
		//		FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);

		//// Get the direction of the x-axis corresponding to the rotation of the
		//// muzzle tip socket
		//FVector MuzzleX(
		//	FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator())
		//		.GetUnitAxis(EAxis::X));

		//// Debug line drawn along the direction of the muzzle tip
		//DrawDebugLine(
		//	GetWorld(), 
		//	MuzzleTipTransform.GetLocation(),
		//	MuzzleTipTransform.GetLocation() + (MuzzleX * 1000.0f),
		//	FColor::Red
		//);

		//// Debug line drawn from the muzzle tip to the hit target
		//DrawDebugLine(
		//	GetWorld(),
		//	MuzzleTipTransform.GetLocation(),
		//	BlasterCharacter->GetHitTarget(),
		//	FColor::Cyan
		//);
	}

	// Only use FABRIK, aim offsets or right hand rotation when we're not doing
	// anything
	bShouldUseFABRIK =
		BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	// Disable right hand rotation and aim offsets when we're in the cooldown
	// state
	bShouldUseAimOffsets =
		BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied &&
		!BlasterCharacter->bDisableGameplay;
	bShouldRotateRightHand =
		BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied &&
		!BlasterCharacter->bDisableGameplay;
}
