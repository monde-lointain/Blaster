// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterPlayerController.h"

#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	InitializeHUD();
}

void ABlasterPlayerController::InitializeHUD()
{
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

void ABlasterPlayerController::SetHUDHealth(
	float CurrentHealth, float MaxHealth)
{
	// Make sure we initialize the HUD in case it hasn't been initialized yet
	if (!BlasterHUD)
	{
		InitializeHUD();
	}

	bool bHUDIsValid = BlasterHUD &&
		               BlasterHUD->CharacterOverlay &&
		               BlasterHUD->CharacterOverlay->HealthBar &&
		               BlasterHUD->CharacterOverlay->HealthText;

	if (bHUDIsValid)
	{
		// Display the player's current health on the health bar widget
		const float HealthPercent = CurrentHealth / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);

		// Display the player's current health in the health text widget
		FString HealthText =
			FString::Printf(TEXT("%d"), FMath::CeilToInt(CurrentHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(
			FText::FromString(HealthText));
	}
}
