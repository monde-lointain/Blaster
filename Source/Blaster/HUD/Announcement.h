// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "Announcement.generated.h"

class UTextBlock;

/**
 *
 */
UCLASS()
class BLASTER_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Text widget for time remaining until match starts */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WarmupTime;

	/** Text widget with the wait screen announcement */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnnouncementText;

	/** Text widget with the wait screen controls */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InfoText;

	/** Text widget displayed after the match with the match result */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResultText;

	/** Text widget displayed after the match with the match winners */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WinnerText;

	/** The color of the result or winner text */
	UPROPERTY(BlueprintReadOnly)
	FLinearColor ResultTextColor;
};
