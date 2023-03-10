// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelStartTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update the warmup time before we start the match
	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime =
			WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartTime;

		// Start the match when we end the countdown
		if (CountdownTime <= 0.0f)
		{
			StartMatch();
		}
	}
	// Track the match time to see when we need to display the cooldown screen
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = (WarmupTime + MatchTime)
						- GetWorld()->GetTimeSeconds() + LevelStartTime;

		// End the match when the timer hits zero
		if (CountdownTime <= 0.0f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	// Update the cooldown time after the match ends
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = (CooldownTime + WarmupTime + MatchTime)
			            - GetWorld()->GetTimeSeconds() 
			            + LevelStartTime;

		// Start the match again when the timer hits zero
		if (CountdownTime <= 0.0f)
		{
			RestartGame();
		}
	}
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	// Get all player controllers in the game and update the current match state
	// on them
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();
		 It;
		 It++)
	{
		ABlasterPlayerController* BlasterPlayer =
			Cast<ABlasterPlayerController>(*It);
		if (BlasterPlayer)
		{
			BlasterPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGameMode::PlayerEliminated(
	ABlasterCharacter* EliminatedCharacter,
	ABlasterPlayerController* EliminatedController,
	ABlasterPlayerController* AttackerController)
{
	// Get the player states for the attacker and the eliminated player
	ABlasterPlayerState* AttackerPlayerState =
		AttackerController
			? Cast<ABlasterPlayerState>(AttackerController->PlayerState)
			: nullptr;
	ABlasterPlayerState* EliminatedPlayerState =
		EliminatedController
		? Cast<ABlasterPlayerState>(EliminatedController->PlayerState)
		: nullptr;

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	// Increment the attacking player's score if they didn't eliminate
	// themselves and check to see if they're the new top scorer
	if (AttackerPlayerState && 
		AttackerPlayerState != EliminatedPlayerState &&
		BlasterGameState)
	{
		AttackerPlayerState->AddToScore(1.0f);
		BlasterGameState->UpdateTopScore(AttackerPlayerState);
	}

	// Increment the eliminated player's elim count
	if (EliminatedPlayerState)
	{
		EliminatedPlayerState->AddToElimCount(1);
	}

	if (EliminatedCharacter)
	{
		// Notify the server to eliminate the character. The server will call
		// the multicast RPC to replicate the player elimination down to all
		// clients
		EliminatedCharacter->Eliminated();
	}
}

void ABlasterGameMode::RequestRespawn(
	ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if (EliminatedCharacter)
	{
		// Detatch the controller from the character and set the player state to
		// inactive
		EliminatedCharacter->Reset();

		// Destroy the character
		EliminatedCharacter->Destroy();
	}

	if (EliminatedController)
	{
		// Get all player start actors in the level
		TArray<AActor*> PlayerStartActors;
		UGameplayStatics::GetAllActorsOfClass(
			this, APlayerStart::StaticClass(), PlayerStartActors);

		// Get a random player start from the array of player starts
		int32 RespawnPlayerStartIndex =
			FMath::RandRange(0, PlayerStartActors.Num() - 1);

		// Respawn the player at the random player start location
		RestartPlayerAtPlayerStart(
			EliminatedController, PlayerStartActors[RespawnPlayerStartIndex]);
	}
}
