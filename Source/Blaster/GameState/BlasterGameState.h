// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "BlasterGameState.generated.h"

class ABlasterPlayerState;

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()

public:
	/** Sets the properties for lifetime server replication */
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateTopScore(ABlasterPlayerState* TopScoringPlayer);

	/** The current top score */
	float TopScore = 0.0f;

	/** Tracks the top scoring players */
	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayers;
};
