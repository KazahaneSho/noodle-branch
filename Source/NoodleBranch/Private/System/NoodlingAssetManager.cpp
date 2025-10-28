// Fill out your copyright notice in the Description page of Project Settings.


#include "System/NoodlingAssetManager.h"
#include "Character/NoodlingPawnData.h"
#include "NoodleBranch/NoodlingLogChannels.h"

const FName FNoodlingBundles::Equipped("Equipped");

UNoodlingAssetManager::UNoodlingAssetManager()
{
	DefaultPawnData = nullptr;
}

UNoodlingAssetManager& UNoodlingAssetManager::Get()
{
	check(GEngine);

	if (UNoodlingAssetManager* Singleton = Cast<UNoodlingAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}

	UE_LOG(NoodleBranch_Log,
		Fatal,
		TEXT("Invalid AssetManagerClassName in DefaultEngine.ini. It must be set to NoodlingAssetManager!"));

	// Fatal Error above prevents this from being called.
	return *NewObject<UNoodlingAssetManager>();
}

const UNoodlingPawnData* UNoodlingAssetManager::GetDefaultPawnData() const
{
	return GetAsset(DefaultPawnData);
}

UObject* UNoodlingAssetManager::SynchronousLoadAsset(const FSoftObjectPath& AssetPath)
{
	if (AssetPath.IsValid())
	{
		// TODO: Finish Implementation
		TUniquePtr<FScopeLogTime> LogTimePtr = MakeUnique<FScopeLogTime>(*FString::Printf(
			TEXT("Synchronously loaded asset [%s]"), *AssetPath.ToString()), nullptr, FScopeLogTime::ScopeLog_Seconds);

		if (UAssetManager::IsInitialized())
		{
			return UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath, false);
		}

		// Use LoadObject if AssetManager isn't ready yet.
		return AssetPath.TryLoad();
	}	

	return nullptr;
}

void UNoodlingAssetManager::AddLoadedAsset(const UObject* Asset)
{
	// TODO: Implement
}
