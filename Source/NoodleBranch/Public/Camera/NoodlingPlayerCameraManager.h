// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "NoodlingPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class NOODLEBRANCH_API ANoodlingPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	explicit ANoodlingPlayerCameraManager(const FObjectInitializer& ObjectInitializer);
};
