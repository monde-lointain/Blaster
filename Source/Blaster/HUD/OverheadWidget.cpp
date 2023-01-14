// Fill out your copyright notice in the Description page of Project Settings.

#include "OverheadWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole LocalRole = InPawn->GetLocalRole();

	// Create a string based on the current role the pawn has
	FString Role;

	switch (LocalRole)
	{
		case ENetRole::ROLE_Authority:
		{
			Role = FString("Authority");
			break;
		}
		case ENetRole::ROLE_AutonomousProxy:
		{
			Role = FString("Autonomous Proxy");
			break;
		}
		case ENetRole::ROLE_SimulatedProxy:
		{
			Role = FString("Simulated Proxy");
			break;
		}
		case ENetRole::ROLE_None:
		{
			Role = FString("None");
			break;
		}
	}

	FString LocalRoleString;

	APlayerState* PlayerState = InPawn->GetPlayerState<APlayerState>();

	if (PlayerState)
	{
		FString PlayerName = PlayerState->GetPlayerName();

		LocalRoleString = FString::Printf(
			TEXT("Player Name: %s\nLocal Role: %s"), *PlayerName, *Role);
		SetDisplayText(LocalRoleString);

		return;
	}

	LocalRoleString = FString::Printf(TEXT("Local Role: %s"), *Role);

	// Set the player overhead widget to display their current role
	SetDisplayText(LocalRoleString);
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}
