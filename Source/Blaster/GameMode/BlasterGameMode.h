// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "BlasterGameMode.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;

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

	/** Time to wait before starting the match */
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.0f;

	/** Time to wait before starting the match */
	float LevelStartTime = 0.0f;

protected:
	/** Called when the game starts */
	virtual void BeginPlay() override;

	/** Handles changes in the current match state */
	virtual void OnMatchStateSet() override;

private:
	/** Timer for the warmup counter */
	float CountdownTime = 0.0f;
};
