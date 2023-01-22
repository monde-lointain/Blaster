// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterPlayerState.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

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


