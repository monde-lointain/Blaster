// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "BlasterGameMode.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;

namespace MatchState
{
	// Match has ended. Displaying information about who won and starting the
    // cooldown timer
	extern BLASTER_API const FName Cooldown;
}

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	/** Sets default values */
	ABlasterGameMode();

	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	/** The game mode handling function for player elimination */
	virtual void PlayerEliminated(
		ABlasterCharacter* EliminatedCharacter,
		ABlasterPlayerController* EliminatedController,
		ABlasterPlayerController* AttackerController
	);

	/** Handles player respawning */
	virtual void RequestRespawn(
		ACharacter* EliminatedCharacter, AController* EliminatedController);

	/** The total time the match takes. */
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.0f;

	/** Time to wait before starting the match */
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.0f;

	/** Time to wait after ending the match */
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.0f;

	/** Time in seconds in which the level was loaded */
	float LevelStartTime = 0.0f;

protected:
	/** Called when the game starts */
	virtual void BeginPlay() override;

	/** Handles changes in the current match state */
	virtual void OnMatchStateSet() override;

private:
	/** Timer for the warmup counter */
	float CountdownTime = 0.0f;

public:
	FORCEINLINE float GetCountdownTime() const
	{
		return CountdownTime;
	}
};
