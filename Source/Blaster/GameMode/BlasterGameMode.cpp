// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

void ABlasterGameMode::PlayerEliminated(
	ABlasterCharacter* EliminatedCharacter,
	ABlasterPlayerController* EliminatedController,
	ABlasterPlayerController* AttackerController)
{
	if (EliminatedCharacter)
	{
		// Play the eliminated animation montage
		EliminatedCharacter->Eliminated();
	}
}
