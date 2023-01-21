// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void ABlasterGameMode::PlayerEliminated(
	ABlasterCharacter* EliminatedCharacter,
	ABlasterPlayerController* EliminatedController,
	ABlasterPlayerController* AttackerController)
{
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
