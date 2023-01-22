// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterPlayerState.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Register the player elim count to be replicated by the server
	DOREPLIFETIME(ABlasterPlayerState, ElimCount);
}

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	float CurrentScore = GetScore() + ScoreAmount;
	SetScore(CurrentScore);
	SetScoreOnPlayerHUD();
}

void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	SetScoreOnPlayerHUD();
}

void ABlasterPlayerState::SetScoreOnPlayerHUD()
{
	// Check to make sure the character and the controller are valid, then
	// update the HUD with the current score
	if (!Character)
	{
		Character = Cast<ABlasterCharacter>(GetPawn());
	}
	if (Character)
	{
		if (!Controller)
		{
			Controller = Cast<ABlasterPlayerController>(Character->Controller);
		}
		if (Controller)
		{
			float CurrentScore = GetScore();
			Controller->SetHUDScore(CurrentScore);
		}
	}
}

void ABlasterPlayerState::AddToElimCount(int32 ElimAmount)
{
	ElimCount += ElimAmount;
	SetElimCountOnPlayerHUD();
}

void ABlasterPlayerState::OnRep_ElimCount()
{
	SetElimCountOnPlayerHUD();
}

void ABlasterPlayerState::SetElimCountOnPlayerHUD()
{
	// Check to make sure the character and the controller are valid, then
	// update the HUD with the current elim count
	if (!Character)
	{
		Character = Cast<ABlasterCharacter>(GetPawn());
	}
	if (Character)
	{
		if (!Controller)
		{
			Controller = Cast<ABlasterPlayerController>(Character->Controller);
		}
		if (Controller)
		{
			Controller->SetHUDElimCounter(ElimCount);
		}
	}
}


