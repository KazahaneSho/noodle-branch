// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NoodlingUserFacingExperienceDefinition.generated.h"

class UCommonSession_HostSessionRequest;
/**
 * Description of settings used to display experiences in the UI and start a new session
 */
UCLASS(BlueprintType)
class UNoodlingUserFacingExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/** If true, this is a default experience that should be used for quick play and given priority in the UI */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Experience)
	bool bIsDefaultExperience = false;

public:

	/** Create a request object that is used to actually start a session with these settings */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta = (WorldContext = "WorldContextObject"))
	UCommonSession_HostSessionRequest* CreateHostingRequest(const UObject* WorldContextObject) const;
};
