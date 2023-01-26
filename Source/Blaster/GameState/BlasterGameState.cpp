// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterGameState.h"

#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Net/UnrealNetwork.h"

void ABlasterGameState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Register the array of player states to be replicated by the server
	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
}

void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* TopScoringPlayer)
{
	if (TopScoringPlayers.IsEmpty())
	{
		TopScoringPlayers.Add(TopScoringPlayer);
		TopScore = TopScoringPlayer->GetScore();
	}
	// If we have two players tied at the top score
	else if (TopScoringPlayer->GetScore() == TopScore)
	{
		// Only add the player to the array if they aren't there already
		TopScoringPlayers.AddUnique(TopScoringPlayer);
	}
	// If someone else took the lead
	else if (TopScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(TopScoringPlayer);
		TopScore = TopScoringPlayer->GetScore();
	}
}
