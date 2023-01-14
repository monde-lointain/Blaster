// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

class USphereComponent;
class UWidgetComponent;

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

	/** Turns the widget displayed when overlapping a weapon on and off */
	void ShowPickupWidget(bool bShowWidget);

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

public:
	/** Handles changes in the weapon state for the server */
	void SetWeaponState(EWeaponState State);

	FORCEINLINE USphereComponent* GetAreaSphere() const
	{
		return AreaSphere;
	}
};