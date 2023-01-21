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
	void SetHUDHealth(float CurrentHealth, float MaxHealth);

protected:
	virtual void BeginPlay() override;

private:
	/** Represents the HUD for the character */
	ABlasterHUD* BlasterHUD;

	/** Initializes the HUD */
	void InitializeHUD();
};
