// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();

	if (PlayerState)
	{
		FString PlayerName = PlayerState->GetPlayerName();

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Cyan,
				FString::Printf(TEXT("%s joined the game!"), *PlayerName)
			);
		}
	}

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	if (NumberOfPlayers == 2)
	{
		UWorld* World = GetWorld();

		if (World)
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
		}
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	APlayerState* PlayerState = Exiting->GetPlayerState<APlayerState>();

	if (PlayerState)
	{
		FString PlayerName = PlayerState->GetPlayerName();

		GEngine->AddOnScreenDebugMessage(
			-1,
			15.0f,
			FColor::Red,
			FString::Printf(TEXT("%s left the game"), *PlayerName)
		);
	}
}
