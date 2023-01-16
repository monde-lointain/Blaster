// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "CombatComponent.generated.h"

class AWeapon;
class ABlasterCharacter;

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

private:
	ABlasterCharacter* Character;

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
};
