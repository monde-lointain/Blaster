// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "BlasterPlayerController.generated.h"

class ABlasterHUD;

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** Updates the health bar on the HUD widget */
	void SetHUDHealth(float CurrentHealth, float MaxHealth);

	/** Updates the score counter on the HUD widget */
	void SetHUDScore(float Score);

	/** Updates the elim counter on the HUD widget */
	void SetHUDElimCounter(int32 ElimCount);

	/** Updates the weapon ammo counter on the HUD widget */
	void SetHUDWeaponAmmo(int32 Ammo);

	/** Updates the carried ammo counter on the HUD widget */
	void SetHUDCarriedAmmo(int32 Ammo);

	/** Updates the time remaining counter on the HUD */
	void SetHUDRemainingMatchTime(float RemainingTime);

	/** Called when a player possesses a pawn */
	virtual void OnPossess(APawn* InPawn) override;

	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	virtual void SetHUDTime();

private:
	/** Represents the HUD for the character */
	UPROPERTY()
	ABlasterHUD* BlasterHUD;

	float MatchTime = 120.0f;
	uint32 CountdownInt = 0;
};
