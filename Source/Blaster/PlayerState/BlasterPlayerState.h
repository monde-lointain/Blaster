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
	/** Sets the properties for lifetime server replication */
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Updates the player's current score */
	void AddToScore(float ScoreAmount);

	/** Updates the player's current elim count */
	void AddToElimCount(int32 ElimAmount);

	/** Replication notify for score */
	virtual void OnRep_Score() override;

	/** Replication notify for elimination count */
	UFUNCTION()
	virtual void OnRep_ElimCount();

	/** Updates the score counter on a player's HUD */
	void SetScoreOnPlayerHUD();

	/** Updates the elim counter on a player's HUD */
	void SetElimCountOnPlayerHUD();

private:
	/** The character using this player state class */
	UPROPERTY()
	ABlasterCharacter* Character;

	/** The controller using this player state class */
	UPROPERTY()
	ABlasterPlayerController* Controller;

	/** The number of times the player has been eliminated */
	UPROPERTY(ReplicatedUsing = OnRep_ElimCount)
	int32 ElimCount;
};
