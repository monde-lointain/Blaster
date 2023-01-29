// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterHUD.h"

#include "Announcement.h"
#include "CharacterOverlay.h"
#include "SniperScope.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ABlasterHUD::AddCharacterOverlay()
{
	// Check to see if the player controller and the character overlay class are
	// valid, then create the character overlay and add it to the viewport
	APlayerController* PlayerController = GetOwningPlayerController();

	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(
			PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::AddAnnouncement()
{
	// Check to see if the player controller and the announcement class are
	// valid, then create the announcement UI and add it to the viewport
	APlayerController* PlayerController = GetOwningPlayerController();

	if (PlayerController && AnnouncementClass)
	{
		Announcement =
			CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void ABlasterHUD::AddSniperScope()
{
	// Check to see if the player controller and the sniper scope class are
	// valid, then create the sniper scope overlay and add it to the viewport
	APlayerController* PlayerController = GetOwningPlayerController();

	if (PlayerController && SniperScopeClass)
	{
		SniperScope =
			CreateWidget<USniperScope>(PlayerController, SniperScopeClass);
		SniperScope->AddToViewport();
	}
}

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;

	if (GEngine)
	{
		// Get the window center coordinates where we'll draw the crosshairs
		GEngine->GameViewport->GetViewportSize(ViewportSize);

		const FVector2D ViewportCenter(
			ViewportSize.X / 2.0f, 
			ViewportSize.Y / 2.0f
		);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		/**
		 * Check to see if each crosshair texture is set, then if set, calculate
		 * the position to draw each part of the crosshairs at and then draw
		 * them
		 */
		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.0f, 0.0f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread,
				HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.0f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread,
				HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.0f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread,
				HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			FVector2D Spread(0.0f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread,
				HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread(0.0f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread,
				HUDPackage.CrosshairsColor);
		}
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter,
	FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();

	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.0f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.0f) + Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X, TextureDrawPoint.Y,
		TextureWidth, TextureHeight,
		0.0f, 0.0f,
		1.0f, 1.0f,
		CrosshairColor
	);
}
