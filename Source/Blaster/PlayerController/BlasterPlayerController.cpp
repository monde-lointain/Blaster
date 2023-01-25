// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterPlayerController.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/HUD/Announcement.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());

	// Create the announcement UI to show that we're waiting for the match to
	// start
	if (BlasterHUD)
	{
		BlasterHUD->AddAnnouncement();
	}
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Register the match state to be replicated by the server
	DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
}

void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSinceLastSync += DeltaTime;
	// Sync up with the server at the designated sync interval
	if (IsLocalController() && TimeSinceLastSync > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetCurrentLocalTime());
		TimeSinceLastSync = 0.0f;
	}
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Reset the player's health
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);

	if (BlasterCharacter)
	{
		SetHUDHealth(
			BlasterCharacter->CurrentHealth, BlasterCharacter->MaxHealth);
	}
}

void ABlasterPlayerController::SetHUDHealth(
	float CurrentHealth, float MaxHealth)
{
	// Make sure we initialize the HUD in case it hasn't been initialized yet
	if (!BlasterHUD)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	}

	bool bHUDIsValid = BlasterHUD &&
		               BlasterHUD->CharacterOverlay &&
		               BlasterHUD->CharacterOverlay->HealthBar &&
		               BlasterHUD->CharacterOverlay->HealthText;

	if (bHUDIsValid)
	{
		// Display the player's current health on the health bar widget
		const float HealthPercent = CurrentHealth / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);

		// Display the player's current health in the health text widget
		FString HealthText =
			FString::Printf(TEXT("%d"), FMath::CeilToInt(CurrentHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(
			FText::FromString(HealthText));
	}
	// Cache the health to be initialized later if the HUD hasn't been set yet
	else
	{
		bCharacterOverlayNeedsInit = true;
		HUDHealth = CurrentHealth;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	// Make sure we initialize the HUD in case it hasn't been initialized yet
	if (!BlasterHUD)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	}

	bool bHUDIsValid = BlasterHUD &&
		               BlasterHUD->CharacterOverlay &&
		               BlasterHUD->CharacterOverlay->ScoreCounter;

	if (bHUDIsValid)
	{
		// Display the player's current score in the score counter widget
		FString ScoreText =
			FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreCounter->SetText(
			FText::FromString(ScoreText));
	}
	// Cache the score to be initialized later if the HUD hasn't been set yet
	else
	{
		bCharacterOverlayNeedsInit = true;
		HUDScore = Score;
	}
}

void ABlasterPlayerController::SetHUDElimCounter(int32 ElimCount)
{
	// Make sure we initialize the HUD in case it hasn't been initialized yet
	if (!BlasterHUD)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	}

	bool bHUDIsValid = BlasterHUD &&
		               BlasterHUD->CharacterOverlay &&
		               BlasterHUD->CharacterOverlay->ElimCounter;

	if (bHUDIsValid)
	{
		// Display the player's current elim count
		FString ElimCountText = FString::Printf(TEXT("%d"), ElimCount);
		BlasterHUD->CharacterOverlay->ElimCounter->SetText(
			FText::FromString(ElimCountText));
	}
	// Cache the elim count to be initialized later if the HUD hasn't been set yet
	else
	{
		bCharacterOverlayNeedsInit = true;
		HUDElims = ElimCount;
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	// Make sure we initialize the HUD in case it hasn't been initialized yet
	if (!BlasterHUD)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	}

	bool bHUDIsValid = BlasterHUD && 
		               BlasterHUD->CharacterOverlay &&
					   BlasterHUD->CharacterOverlay->WeaponAmmoCounter;

	if (bHUDIsValid)
	{
		// Display the current weapon ammo
		FString AmmoCountText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoCounter->SetText(
			FText::FromString(AmmoCountText));
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	// Make sure we initialize the HUD in case it hasn't been initialized yet
	if (!BlasterHUD)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	}

	bool bHUDIsValid = BlasterHUD &&
		               BlasterHUD->CharacterOverlay &&
		               BlasterHUD->CharacterOverlay->CarriedAmmoCounter;

	if (bHUDIsValid)
	{
		// Display the player's current carried ammo
		FString AmmoCountText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoCounter->SetText(
			FText::FromString(AmmoCountText));
	}
}

void ABlasterPlayerController::SetHUDRemainingMatchTime(float RemainingTime)
{
	// Make sure we initialize the HUD in case it hasn't been initialized yet
	if (!BlasterHUD)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	}

	bool bHUDIsValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchTimerText;

	if (bHUDIsValid)
	{
		// Calculate the minutes and seconds remaining in the match, then update
		// the match countdown timer
		int32 Minutes = FMath::FloorToInt(RemainingTime / 60.0f);
		int32 Seconds = FMath::FloorToInt(RemainingTime - (Minutes * 60));

		FString RemainingTimeText =
			FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchTimerText->SetText(
			FText::FromString(RemainingTimeText));
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	// Sync up the client and server times
	uint32 SecondsRemaining = FMath::CeilToInt(MatchTime - GetServerTime());

	if (CountdownInt != SecondsRemaining)
	{
		SetHUDRemainingMatchTime(MatchTime - GetServerTime());
	}
	CountdownInt = SecondsRemaining;
}

void ABlasterPlayerController::PollInit()
{
	// Check if the character overlay hasn't been initialized yet, and if not,
	// then get it from the HUD
	if (!CharacterOverlay)
	{
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;

			// Initialize the HUD time and score counters when the overlay is
			// ready
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDElimCounter(HUDElims);
			}
		}
	}
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(
	float TimeOfClientRequest)
{
	// Set the time we recieved the message at
	float ServerTimeOfReciept = GetCurrentLocalTime();
	// Send back to the client the time the client sent the message to the
	// server at, and the time the server recieved the client message
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReciept);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(
	float TimeOfClientRequest, float TimeServerRecievedClientRequest)
{
	float RoundTripTime = GetCurrentLocalTime() - TimeOfClientRequest;
	// Make an estimate of the current server time by assuming the time taken
	// from the message to travel from the server to the client was exactly 1/2
	// the round trip time
	float ServerCurrentTime = (0.5f * RoundTripTime) + TimeServerRecievedClientRequest;
	ClientToServerDeltaTime = ServerCurrentTime - GetCurrentLocalTime();
}

float ABlasterPlayerController::GetCurrentLocalTime()
{
	return GetWorld()->GetTimeSeconds();
}

float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		return GetCurrentLocalTime();
	}
	else
	{
		return GetCurrentLocalTime() + ClientToServerDeltaTime;
	}
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetCurrentLocalTime());
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	// Set the announcement HUD if we're waiting to start
	if (MatchState == MatchState::WaitingToStart)
	{

	}
	// Set the character overlays when the match starts
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	// Set the character overlays when the match starts
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void ABlasterPlayerController::HandleMatchHasStarted()
{
	// Make sure we initialize the HUD in case it hasn't been initialized yet
	if (!BlasterHUD)
	{
		BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	}
	if (BlasterHUD)
	{
		BlasterHUD->AddCharacterOverlay();

		// Hide the announcement UI
		if (BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
