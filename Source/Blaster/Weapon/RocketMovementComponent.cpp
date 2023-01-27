// Fill out your copyright notice in the Description page of Project Settings.

#include "RocketMovementComponent.h"

URocketMovementComponent::EHandleBlockingHitResult
URocketMovementComponent::HandleBlockingHit(const FHitResult& Hit,
	float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);

	// We always want to advance to the next substep when calling this function,
	// that way the rocket can continue to move even when hitting the owner
	// actor
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void URocketMovementComponent::HandleImpact(
	const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	// Rockets should not stop, only explode when their collision box detects a
	// hit
}
