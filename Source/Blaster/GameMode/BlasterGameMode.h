// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "BlasterGameMode.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	/** The game mode handling function for player elimination */
	virtual void PlayerEliminated(
		ABlasterCharacter* EliminatedCharacter,
		ABlasterPlayerController* EliminatedController,
		ABlasterPlayerController* AttackerController
	);

	virtual void RequestRespawn(
		ACharacter* EliminatedCharacter, AController* EliminatedController);
};
