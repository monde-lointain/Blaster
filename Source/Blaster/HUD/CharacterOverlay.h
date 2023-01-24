// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "CharacterOverlay.generated.h"

class UProgressBar;
class UTextBlock;

/**
 *
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Player health bar widget */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	/** Player current health text widget */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	/** Player score counter text widget */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreCounter;

	/** Player elimination counter text widget */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ElimCounter;

	/** Text widget for the ammo on the weapon */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoCounter;

	/** Text widget for the ammo the player is carrying */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoCounter;

	/** Text widget for the remaining match time */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchTimerText;
};
