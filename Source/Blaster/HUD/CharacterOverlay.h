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
};
