// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "SniperScope.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API USniperScope : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* ScopeZoomIn;
};
