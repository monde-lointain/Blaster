// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h" 

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnFindSessionsComplete)),
	JoinSessionsCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnStartSessionComplete))
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();

	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Green,
				FString::Printf(TEXT("Found subsystem %s"),
					*Subsystem->GetSubsystemName().ToString())
			);
		}
	}
}

void UMultiplayerSessionsSubsystem::CreateSession(
	int32 NumPublicConnections, FString MatchType)
{
	// If the session interface isn't valid for whatever reason, that means we
	// can't make call the function we're trying to call. We'll have to return
	// early and broadcast a message that the call failed.
	if (!SessionInterface.IsValid())
	{
		return;
	}

	// Check if there's already an existing session
	FNamedOnlineSession* ExistingSession =
		SessionInterface->GetNamedSession(NAME_GameSession);

	if (ExistingSession)
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;

		DestroySession();
	}

	// Set the delegate to the handle of the session interface
	CreateSessionCompleteDelegateHandle =
		SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
			CreateSessionCompleteDelegate);

	// Fill in all the session settings we want to use
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch =
		IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->Set(FName("MatchType"), MatchType,
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->BuildUniqueId = 1;
	LastSessionSettings->bUseLobbiesIfAvailable = true;

	const ULocalPlayer* LocalPlayer =
		GetWorld()->GetFirstLocalPlayerFromController();

	bool bSessionCreationSuccessful =
		SessionInterface->CreateSession(
			*LocalPlayer->GetPreferredUniqueNetId(),
			NAME_GameSession, 
			*LastSessionSettings
		);

	// If the call fails, clear the delegate handle and broadcast an error
	// result
	if (!bSessionCreationSuccessful)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(
			CreateSessionCompleteDelegateHandle);

		MultiplayerOnCreateSessionComplete.Broadcast(false);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Red,
				FString(TEXT("Failed to create session"))
			);
		}
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults) 
{
	// If the session interface isn't valid for whatever reason, that means we
	// can't make call the function we're trying to call. We'll have to return
	// early and broadcast a message that the call failed.
	if (!SessionInterface.IsValid())
	{
		return;
	}

	// Set the delegate to the handle of the session interface
	FindSessionsCompleteDelegateHandle =
		SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
			FindSessionsCompleteDelegate);

	// Fill in all the search settings we want to useS
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery =
		IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSearch->QuerySettings.Set(
		SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer =
		GetWorld()->GetFirstLocalPlayerFromController();

	bool bFoundSessions = SessionInterface->FindSessions(
		*LocalPlayer->GetPreferredUniqueNetId(),
		LastSessionSearch.ToSharedRef()
	);

	// If the call fails, clear the delegate handle and broadcast an error
	// result
	if (!bFoundSessions)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(
			FindSessionsCompleteDelegateHandle);

		MultiplayerOnFindSessionsComplete.Broadcast(
			TArray<FOnlineSessionSearchResult>(), false);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Red,
				FString(TEXT("Failed to find session"))
			);
		}
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(
	const FOnlineSessionSearchResult& SessionResult)
{
	// If the session interface isn't valid for whatever reason, that means we
	// can't make call the function we're trying to call. We'll have to return
	// early and broadcast a message that the call failed.
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnJoinSessionComplete.Broadcast(
			EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	// Set the delegate to the handle of the session interface
	JoinSessionsCompleteDelegateHandle =
		SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
			JoinSessionsCompleteDelegate);

	const ULocalPlayer* LocalPlayer =
		GetWorld()->GetFirstLocalPlayerFromController();

	bool bJoinedSession =
		SessionInterface->JoinSession(
			*LocalPlayer->GetPreferredUniqueNetId(),
			NAME_GameSession, 
			SessionResult
		);

	// If the call fails, clear the delegate handle and broadcast an error
	// result
	if (!bJoinedSession)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(
			JoinSessionsCompleteDelegateHandle);

		MultiplayerOnJoinSessionComplete.Broadcast(
			EOnJoinSessionCompleteResult::UnknownError);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Red,
				FString(TEXT("Failed to join session"))
			);
		}
	}
}

void UMultiplayerSessionsSubsystem::DestroySession() 
{
	// If the session interface isn't valid for whatever reason, that means we
	// can't make call the function we're trying to call. We'll have to return
	// early and broadcast a message that the call failed.
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	// Set the delegate to the handle of the session interface
	DestroySessionCompleteDelegateHandle =
		SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
			DestroySessionCompleteDelegate);

	bool bDestroyedSession = SessionInterface->DestroySession(NAME_GameSession);

	// If the call fails, clear the delegate handle and broadcast an error
	// result
	if (!bDestroyedSession)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(
			DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Red,
				FString(TEXT("Failed to destroy session"))
			);
		}
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
	// If the session interface isn't valid for whatever reason, that means we
	// can't make call the function we're trying to call. We'll have to return
	// early and broadcast a message that the call failed.
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnStartSessionComplete.Broadcast(false);
		return;
	}

	// Set the delegate to the handle of the session interface
	StartSessionCompleteDelegateHandle =
		SessionInterface->AddOnStartSessionCompleteDelegate_Handle(
			StartSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer =
		GetWorld()->GetFirstLocalPlayerFromController();

	bool bStartedSession = SessionInterface->StartSession(NAME_GameSession);

	// If the call fails, clear the delegate handle and broadcast an error
	// result
	if (!bStartedSession)
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(
			StartSessionCompleteDelegateHandle);

		MultiplayerOnStartSessionComplete.Broadcast(false);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.0f,
				FColor::Red,
				FString(TEXT("Failed to start session"))
			);
		}
	}
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(
	FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		// Clear the session interface delegate handle, since we finished this
		// call
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(
			CreateSessionCompleteDelegateHandle);
	}

	// Broadcast using our delegate whether it was successful or not
	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		// Clear the session interface delegate handle, since we finished this
		// call
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(
			FindSessionsCompleteDelegateHandle);
	}

	// If the search was successful but no sessions were found
	if (LastSessionSearch->SearchResults.Num() <= 0)
	{
		MultiplayerOnFindSessionsComplete.Broadcast(
			TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	// Broadcast using our delegate the list of search results, and whether the
	// search was successful or not
	MultiplayerOnFindSessionsComplete.Broadcast(
		LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(
	FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid())
	{
		// Clear the session interface delegate handle, since we finished this
		// call
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(
			JoinSessionsCompleteDelegateHandle);

		// Broadcast using our delegate twhether we have succesfully joined a
		// session or not
		MultiplayerOnJoinSessionComplete.Broadcast(Result);
	}
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(
	FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		// Clear the session interface delegate handle, since we finished this
		// call
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(
			DestroySessionCompleteDelegateHandle);
	}

	// In the case that we are trying to create a session when a session already
	// exists, we'll destroy the existing session here and create a new oneS
	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}

	// Broadcast using our delegate whether it was successful or not
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(
	FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		// Clear the session interface delegate handle, since we finished this
		// call
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(
			StartSessionCompleteDelegateHandle);
	}

	// Broadcast using our delegate whether it was successful or not
	MultiplayerOnStartSessionComplete.Broadcast(bWasSuccessful);
}
