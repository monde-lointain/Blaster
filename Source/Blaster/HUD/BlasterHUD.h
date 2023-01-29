// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "BlasterHUD.generated.h"

class UTexture2D;
class UCharacterOverlay;
class UAnnouncement;
class USniperScope;

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	/** Textures for each individual part of the crosshairs */
	UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;

	/**
	 * Indicates how much the crosshairs should be spread apart at any given
	 * moment. Calculated based on the player's velocity in CombatComponent
	 */
	float CrosshairSpread;

	/** Current color of the crosshairs */
	FLinearColor CrosshairsColor;
};

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	/** HUD widgets representing the player's character overlay */
	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	/** Announcement widgets for the waiting screen */
	UPROPERTY()
	UAnnouncement* Announcement;

	/** Widget for the sniper scope overlay */
	UPROPERTY()
	USniperScope* SniperScope;

	/** Class for the player's character overlay */
	UPROPERTY(EditAnywhere, Category = PlayerStats)
	TSubclassOf<UUserWidget> CharacterOverlayClass;

	/** Class for the announcement widget */
	UPROPERTY(EditAnywhere, Category = Announcements)
	TSubclassOf<UUserWidget> AnnouncementClass;

	/** Class for the announcement widget */
	UPROPERTY(EditAnywhere, Category = SniperScope)
	TSubclassOf<UUserWidget> SniperScopeClass;

	/** Initializes the character overlay */
	void AddCharacterOverlay();

	/** Initializes the announcement UI */
	void AddAnnouncement();

	/** Initializes the sniper scoper overlay */
	void AddSniperScope();

protected:
	virtual void BeginPlay() override;

private:
	/**
	 * Contains all the individual crosshair textures as well as a factor for
	 * how much they should be spread apart
	 */
	FHUDPackage HUDPackage;

	/** Draws a crosshair texture to the appropriate point on the screen */
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter,
		FVector2D Spread, FLinearColor CrosshairColor);

	/** Scale factor for the crosshair spread */
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.0f;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package)
	{
		HUDPackage = Package;
	}
};
