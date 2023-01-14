// Fill out your copyright notice in the Description page of Project Settings.

#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UMenu::MenuSetup(
	int32 NumberOfPublicConnections, FString TypeOfMatch, FString PathToLobby)
{
	LobbyPath = FString::Printf(TEXT("%s?listen"), *PathToLobby);
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;

	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();

		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(
				EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();

	if (GameInstance)
	{
		MultiplayerSessionsSubsystem =
			GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	// Binding the MultiplayerSessionsSubsystem delegates to our callback functions
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete
			.AddDynamic(this, &UMenu::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete
			.AddUObject(this, &UMenu::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete
			.AddUObject(this, &UMenu::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete
			.AddDynamic(this, &UMenu::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete
			.AddDynamic(this, &UMenu::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &UMenu::HostButtonClicked);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UMenu::JoinButtonClicked);
	}

	return true;
}

void UMenu::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Green,
				FString(TEXT("Session created successfully"))
			);
		}

		UWorld* World = GetWorld();

		if (World)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.0f,
					FColor::Yellow,
					FString(TEXT("World found. Server travel started"))
				);
			}

			World->ServerTravel(LobbyPath);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Red,
				FString(TEXT("Failed to create session"))
			);
		}

		HostButton->SetIsEnabled(true);
	}
}

void UMenu::OnFindSessions(
	const TArray<FOnlineSessionSearchResult>& SessionResults,
	bool bWasSuccessful)
{
	if (!MultiplayerSessionsSubsystem)
	{
		return;
	}

	// Loop through all the search results and join the first session that
	// matches the match type we're searching for
	for (FOnlineSessionSearchResult Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);

		if (SettingsValue == MatchType)
		{
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}

	// If the session search failed or the search didn't return any results
	if (!bWasSuccessful || SessionResults.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();

	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(
				NAME_GameSession, Address);

			APlayerController* PlayerController =
				GetGameInstance()->GetFirstLocalPlayerController();

			if (PlayerController)
			{
				PlayerController->ClientTravel(
					Address, 
					ETravelType::TRAVEL_Absolute
				);
			}
		}
	}

	// If something happened to go wrong and the player couldn't join (maybe
	// there was a session left open with no one in it)
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful) {}

void UMenu::OnStartSession(bool bWasSuccessful) 
{
	//if (bWasSuccessful)
	//{
	//	// Replace NewMap with whatever level we're traveling to next
	//	UGameplayStatics::OpenLevel(GetWorld(), "NewMap", true, "listen");
	//}
}

void UMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.0f,
			FColor::Yellow,
			FString(TEXT("Checking MultiplayerSessionsSubsystem"))
		);
	}

	if (MultiplayerSessionsSubsystem)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Yellow,
				FString(TEXT("MultiplayerSessionsSubsystem found"))
			);
		}

		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();

	UWorld* World = GetWorld();

	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();

		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			// Set to false later
			PlayerController->SetShowMouseCursor(true);
		}
	}
}