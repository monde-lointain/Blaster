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
	/** Sets the health for the HUD widget */
	void SetHUDHealth(float CurrentHealth, float MaxHealth);

	/** Called when a player possesses a pawn */
	virtual void OnPossess(APawn* InPawn) override;

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

private:
	/** Represents the HUD for the character */
	ABlasterHUD* BlasterHUD;
};
