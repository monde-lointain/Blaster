// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"

/**
 *
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem :
	public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Declaring delegates for the menu system to bind callbacks to
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
		FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
	DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete,
		const TArray<FOnlineSessionSearchResult>& SessionResults,
		bool bWasSuccessful);
	DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete,
		EOnJoinSessionCompleteResult::Type Result);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
		FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
		FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);

	UMultiplayerSessionsSubsystem();

	/**
	 * Handles session functionality. The menu class will call these
	 */
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();

	/**
	 * Delegates for the menu system to bind callbacks to
	 */
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;

protected:
	/**
	 * Interal callbackse for the online session interface delegate list. These
	 * don't need to be called outside this class.
	 */
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
	/** The online session interface, which we call all our functions from */
	IOnlineSessionPtr SessionInterface;

	/** Variables for our session settings and our session search results */
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	/** Delegate called when session created */
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;

	/** Handle to registered delegate for creating a session */
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	/** Delegate called when searching for sessions */
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;

	/** Handle to registered delegate for searching for sessions */
	FDelegateHandle FindSessionsCompleteDelegateHandle;

	/** Delegate for joining a session */
	FOnJoinSessionCompleteDelegate JoinSessionsCompleteDelegate;

	/** Handle to registered delegate for joining a session */
	FDelegateHandle JoinSessionsCompleteDelegateHandle;

	/** Delegate called when session started */
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;

	/** Handle to registered delegate for starting a session */
	FDelegateHandle StartSessionCompleteDelegateHandle;

	/** Delegate called when session destroyed */
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;

	/** Handle to registered delegate for destroying a session */
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	/** 
	 * Checked when a session is destroyed to see if a new session should be
	 * created after
	 */
	bool bCreateSessionOnDestroy = false;

	/** Number of public connections that were in the last match */
	int32 LastNumPublicConnections;

	/** Format of the last match */
	FString LastMatchType;
};
