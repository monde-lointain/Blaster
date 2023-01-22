// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "BlasterPlayerState.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	/** Updates the player's current score */
	void AddToScore(float ScoreAmount);

	/** Replication notify for score */
	virtual void OnRep_Score() override;

	/** Updates the score counter on a player's HUD */
	void SetScoreOnPlayerHUD();

private:
	/** The character using this player state class */
	ABlasterCharacter* Character;

	/** The controller using this player state class */
	ABlasterPlayerController* Controller;
};
