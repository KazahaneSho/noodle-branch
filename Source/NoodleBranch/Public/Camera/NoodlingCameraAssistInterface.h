﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NoodlingCameraAssistInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UNoodlingCameraAssistInterface : public UInterface
{
	GENERATED_BODY()
};


class NOODLEBRANCH_API INoodlingCameraAssistInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * Get the list of actors that we're allowing the camera to penetrate. Useful in 3rd person cameras
	 * when you need the following camera to ignore things like the collection of view targets, the pawn,
	 * a vehicle…etc.
	 */
	virtual void GetIgnoredActorsForCameraPenetration(TArray<const AActor*>& OutActorsAllowPenetration) const { }

	/**
	 * The target actor to prevent penetration on.  Normally, this is almost always the view target, which if
	 * unimplemented will remain true.  However, sometimes the view target, isn't the same as the root actor 
	 * you need to keep in frame.
	 */
	virtual TOptional<AActor*> GetCameraPreventPenetrationTarget() const
	{
		return TOptional<AActor*>();
	}

	/** Called if the camera penetrates the focal target.  Useful if you want to hide the target actor when being overlapped. */
	virtual void OnCameraPenetratingTarget() { }
};
