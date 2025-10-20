// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Templates/SubclassOf.h"
#include "NoodlingAssetManager.generated.h"

#define UE_API NOODLEBRANCH_API

class UNoodlingPawnData;

/**
 * UNoodlingAssetManager
 *
 * Game implementation of the asset manager that overrides functionality and store game-specific types.
 * This class is used by setting 'AssetManagerClassName' to this class in DefaultEngine.ini.
 */
UCLASS(MinimalAPI, Config = Game)
class UNoodlingAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	UE_API UNoodlingAssetManager();

	// Returns the AssetManager Singleton object.
	static UE_API UNoodlingAssetManager& Get();

	template<typename AssetType>
	static AssetType* GetAsset(const TSoftObjectPtr<AssetType>& AssetPtr, bool bKeepInMemory = true);

	UE_API const UNoodlingPawnData* GetDefaultPawnData() const;

protected:

	static UE_API UObject* SynchronousLoadAsset(const FSoftObjectPath& AssetPath);

	UE_API void AddLoadedAsset(const UObject* Asset);
	
protected:
	UPROPERTY(Config)
	TSoftObjectPtr<UNoodlingPawnData> DefaultPawnData;
};

template <typename AssetType>
AssetType* UNoodlingAssetManager::GetAsset(const TSoftObjectPtr<AssetType>& AssetPtr, bool bKeepInMemory)
{
	AssetType* LoadedAsset = nullptr;

	if (const FSoftObjectPath& AssetPath = AssetPtr.ToSoftObjectPath(); AssetPath.IsValid())
	{
		LoadedAsset = AssetPtr.Get();
		if (!LoadedAsset)
		{
			LoadedAsset = Cast<AssetType>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedAsset, TEXT("Failed to load asset [%s]"), *AssetPtr.ToString());
		}

		if (LoadedAsset && bKeepInMemory)
		{
			// Added to Loaded Asset List.
			Get().AddLoadedAsset(Cast<UObject>(LoadedAsset));
		}
	}
	return LoadedAsset;
}
