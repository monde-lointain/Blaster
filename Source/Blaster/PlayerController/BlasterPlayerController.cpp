// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterPlayerController.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Reset the player's health
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);

	if (BlasterCharacter)
	{
		SetHUDHealth(
			BlasterCharacter->CurrentHealth, BlasterCharacter->MaxHealth);
	}
}

void ABlasterPlayerController::SetHUDHealth(
	float CurrentHealth, float MaxHealth)
{
	// Make sure we initialize the HUD in case it hasn't been initialized yet
	if (!BlasterHUD)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
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

void ABlasterPlayerController::SetHUDScore(float Score)
{
	// Make sure we initialize the HUD in case it hasn't been initialized yet
	if (!BlasterHUD)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	}

	bool bHUDIsValid = BlasterHUD &&
		               BlasterHUD->CharacterOverlay &&
		               BlasterHUD->CharacterOverlay->ScoreCounter;

	if (bHUDIsValid)
	{
		// Display the player's current score in the score counter widget
		FString ScoreText =
			FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreCounter->SetText(
			FText::FromString(ScoreText));
	}
}

void ABlasterPlayerController::SetHUDElimCounter(int32 ElimCount)
{
	// Make sure we initialize the HUD in case it hasn't been initialized yet
	if (!BlasterHUD)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	}

	bool bHUDIsValid = BlasterHUD &&
		               BlasterHUD->CharacterOverlay &&
		               BlasterHUD->CharacterOverlay->ElimCounter;

	if (bHUDIsValid)
	{
		// Display the player's current elim count
		FString ElimCountText = FString::Printf(TEXT("%d"), ElimCount);
		BlasterHUD->CharacterOverlay->ElimCounter->SetText(
			FText::FromString(ElimCountText));
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	// Make sure we initialize the HUD in case it hasn't been initialized yet
	if (!BlasterHUD)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	}

	bool bHUDIsValid = BlasterHUD && 
		               BlasterHUD->CharacterOverlay &&
					   BlasterHUD->CharacterOverlay->WeaponAmmoCounter;

	if (bHUDIsValid)
	{
		// Display the current weapon ammo
		FString AmmoCountText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoCounter->SetText(
			FText::FromString(AmmoCountText));
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	// Make sure we initialize the HUD in case it hasn't been initialized yet
	if (!BlasterHUD)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	}

	bool bHUDIsValid = BlasterHUD &&
		               BlasterHUD->CharacterOverlay &&
		               BlasterHUD->CharacterOverlay->CarriedAmmoCounter;

	if (bHUDIsValid)
	{
		// Display the player's current carried ammo
		FString AmmoCountText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoCounter->SetText(
			FText::FromString(AmmoCountText));
	}
}
