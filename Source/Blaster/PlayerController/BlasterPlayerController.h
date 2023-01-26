// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "BlasterPlayerController.generated.h"

class ABlasterHUD;
class ABlasterGameMode;
class UCharacterOverlay;

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** Updates the health bar on the HUD widget */
	void SetHUDHealth(float CurrentHealth, float MaxHealth);

	/**
	 * Sets the color of the health bar widget based on the player's current
	 * health
	 */
	void SetHUDHealthColor(float HealthPercent);

	/** Updates the score counter on the HUD widget */
	void SetHUDScore(float Score);

	/** Updates the elim counter on the HUD widget */
	void SetHUDElimCounter(int32 ElimCount);

	/** Updates the weapon ammo counter on the HUD widget */
	void SetHUDWeaponAmmo(int32 Ammo);

	/** Updates the carried ammo counter on the HUD widget */
	void SetHUDCarriedAmmo(int32 Ammo);

	/** Updates the time remaining counter on the HUD */
	void SetHUDRemainingMatchTime(float RemainingTime);

	/** Updates the time remaining counter on the announcement HUD timer */
	void SetHUDRemainingAnnouncementTime(float RemainingTime);

	/** Called when a player possesses a pawn */
	virtual void OnPossess(APawn* InPawn) override;

	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	/** Sets the properties for lifetime server replication */
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Returns the current time in seconds for the local world clock, whether
	 * it be client or server
	 */
	float GetCurrentLocalTime();

	/** Get the current time synced with the server's world clock */
	virtual float GetServerTime();

	/** Overriden to sync with server clock as soon as possible */
	virtual void ReceivedPlayer() override;

	/** Handles switching between match states from the player controller */
	void OnMatchStateSet(FName State);

	void HandleMatchHasStarted();

	void HandleCooldown();

	void DisplayWinnerText();

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	virtual void SetHUDTime();

	/**
	 * Polls for classes that don't get initialized on the first frame and
	 * initializes them
	 */
	void PollInit();

	/** Syncing time between the client and server */

	/**
	 * Server RPC to request the current server time. The server sends this
	 * back to the client so the client can calculate the round trip time
	 */
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	/**
	 * Client RPC for reporting the current server time in response to
	 * ServerRequestServerTime
	 */
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerRecievedClientRequest);

	/** Indicates the difference in local time between the client and server */
	float ClientToServerDeltaTime;

	/** Time interval to sync client time with the server at */
	UPROPERTY(EditAnywhere, Category  = Time)
	float TimeSyncFrequency = 5.0f;

	/**
	 * The time since the last time the client time was synced with the server
	 * time
	 */
	float TimeSinceLastSync = 0.0f;

	/** Syncs the client and server time at the time sync interval */
	void CheckTimeSync(float DeltaTime);

	/** Server RPC for the match state */
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	/** Client RPC that handles updating the match state upon joining */
	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Cooldown,
		float Match, float StartTime);

private:
	/** Represents the HUD for the character */
	UPROPERTY()
	ABlasterHUD* BlasterHUD;

	/** Represents the game mode this character is currently in */
	UPROPERTY()
	ABlasterGameMode* BlasterGameMode;

	/** The total time the match takes. Retrieved from the game mode */
	float MatchTime = 0.0f;

	/** Time to wait before starting the match. Retrieved from the game mode */
	float WarmupTime = 0.0f;

	/** Time to wait after ending the match. Retrieved from the game mode */
	float CooldownTime = 0.0f;

	/** Time in seconds in which the level was loaded. Retrieved from the game mode */
	float LevelStartTime = 0.0f;

	uint32 CountdownInt = 0;

	/** The current state of the match */
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	/** Replication notify for the match state */
	UFUNCTION()
	void OnRep_MatchState();

	/** Contains the character overlay used by the HUD */
	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	/** Flag for whether the character overlay still needs to be initialized */
	bool bCharacterOverlayNeedsInit = false;

	/** Cached values to set on the overlay when it gets initialized */
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDElims;
	int32 HUDWeaponAmmo;
	int32 HUDCarriedAmmo;
};
