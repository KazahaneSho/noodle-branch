// Fill out your copyright notice in the Description page of Project Settings.


#include "Http/NoodlingGameplayRpcRegistrationComponent.h"

#include "Player/NoodlingPlayerController.h"

UNoodlingGameplayRpcRegistrationComponent* UNoodlingGameplayRpcRegistrationComponent::ObjectInstance = nullptr;
UNoodlingGameplayRpcRegistrationComponent* UNoodlingGameplayRpcRegistrationComponent::GetInstance()
{
#if WITH_RPC_REGISTRY
	if (ObjectInstance == nullptr)
	{
		ObjectInstance = NewObject<UNoodlingGameplayRpcRegistrationComponent>();
		FParse::Value(FCommandLine::Get(), TEXT("externalrpclistenaddress="), ObjectInstance->ListenerAddress);
		FParse::Value(FCommandLine::Get(), TEXT("rpcsenderid="), ObjectInstance->SenderID);
		if (!UExternalRpcRegistry::GetInstance())
		{
			GLog->Log(TEXT("BotRPC"), ELogVerbosity::Warning, FString::Printf(TEXT("Unable to create RPC Registry Instance. This might lead to issues using the RPC Registry.")));
		}
		ObjectInstance->AddToRoot();
	}
#endif
	return ObjectInstance;
}

UWorld* FindGameWorld()
{
	//Find Game World
	if (GEngine->GameViewport)
	{
		UGameInstance* GameInstance = GEngine->GameViewport->GetGameInstance();
		return GameInstance ? GameInstance->GetWorld() : nullptr;
	}
	return GWorld;
}

ANoodlingPlayerController* GetPlayerController()
{
	const UWorld* LocalWorld = FindGameWorld();
	if (!LocalWorld)
	{
		return nullptr;
	}
	//Find PlayerController
	if (ANoodlingPlayerController* PlayerController = Cast<ANoodlingPlayerController>(LocalWorld->GetFirstPlayerController()); !PlayerController)
	{
		return nullptr;
	}
	else
	{
		return PlayerController;
	}
}

TSharedPtr<FJsonObject> UNoodlingGameplayRpcRegistrationComponent::GetJsonObjectFromRequestBody(
	TArray<uint8> InRequestBody)
{
	const FUTF8ToTCHAR WByteBuffer(reinterpret_cast<const ANSICHAR*>(InRequestBody.GetData()), InRequestBody.Num());
	const FString IncomingRequestBody =  FString::ConstructFromPtrSize(WByteBuffer.Get(), WByteBuffer.Length());
	TSharedPtr<FJsonObject> BodyObject = MakeShareable(new FJsonObject());

	if (const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(IncomingRequestBody); FJsonSerializer::Deserialize(JsonReader, BodyObject) && BodyObject.IsValid())
	{
		return BodyObject;
	}

	return nullptr;
}

void UNoodlingGameplayRpcRegistrationComponent::DeregisterHttpCallbacks()
{
	Super::DeregisterHttpCallbacks();
}

void UNoodlingGameplayRpcRegistrationComponent::RegisterAlwaysOnHttpCallbacks()
{
	Super::RegisterAlwaysOnHttpCallbacks();

	const FExternalRpcArgumentDesc CommandDesc(TEXT("command"), TEXT("string"), TEXT("The command to tell the executable to run."));

	RegisterHttpCallback(FName(TEXT("CheatCommand")),
		FHttpPath("/core/cheatcommand"),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateUObject(this, &ThisClass::HttpExecuteCheatCommand),
		true,
		TEXT("Cheats"),
		TEXT("raw"),
		{ CommandDesc });
}

bool UNoodlingGameplayRpcRegistrationComponent::HttpExecuteCheatCommand(const FHttpServerRequest& Request,
	const FHttpResultCallback& OnComplete)
{
	const TSharedPtr<FJsonObject> BodyObject = GetJsonObjectFromRequestBody(Request.Body);

	if (!BodyObject.IsValid())
	{
		TUniquePtr<FHttpServerResponse>Response = CreateSimpleResponse(false, TEXT("Invalid body object"));
		OnComplete(MoveTemp(Response));
		return true;
	}
	if (BodyObject->GetStringField(TEXT("command")).IsEmpty())
	{
		TUniquePtr<FHttpServerResponse>Response = CreateSimpleResponse(false, TEXT("command not found in json body"));
		OnComplete(MoveTemp(Response));
		return true;
	}
	ANoodlingPlayerController* NoodlingPlayerController = GetPlayerController();
	if (!NoodlingPlayerController)
	{
		TUniquePtr<FHttpServerResponse>Response = CreateSimpleResponse(false, TEXT("player controller not found"));
		OnComplete(MoveTemp(Response));
		return true;
	}
	const FString CheatCommand = FString::Printf(TEXT("%s"), *BodyObject->GetStringField(TEXT("command")));
	NoodlingPlayerController->ConsoleCommand(*CheatCommand, true);

	TUniquePtr<FHttpServerResponse>Response = CreateSimpleResponse(true);
	OnComplete(MoveTemp(Response));
	return true;
}

void UNoodlingGameplayRpcRegistrationComponent::RegisterFrontendHttpCallbacks()
{
	// TODO: Add Matchmaking RPCs here
}

void UNoodlingGameplayRpcRegistrationComponent::RegisterInMatchHttpCallbacks()
{
	RegisterHttpCallback(FName(TEXT("GetPlayerStatus")),
		FHttpPath("/player/status"),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateUObject(this, &ThisClass::HttpGetPlayerVitalsCommand),
		true);

	RegisterHttpCallback(FName(TEXT("PlayerFireOnce")),
		FHttpPath("/player/status"),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateUObject(this, &ThisClass::HttpFireOnceCommand),
		true);
}

bool UNoodlingGameplayRpcRegistrationComponent::HttpFireOnceCommand(const FHttpServerRequest& Request,
	const FHttpResultCallback& OnComplete)
{
	const ANoodlingPlayerController* NoodlingPlayerController = GetPlayerController();
	if (!NoodlingPlayerController)
	{
		TUniquePtr<FHttpServerResponse> Response = CreateSimpleResponse(false, TEXT("No player controller found"));
		OnComplete(MoveTemp(Response));
		return true;
	}
	if (const APawn* FortPlayerPawn = NoodlingPlayerController->GetPawn(); !FortPlayerPawn)
	{
		TUniquePtr<FHttpServerResponse> Response = CreateSimpleResponse(false, TEXT("Player pawn not found"));
		OnComplete(MoveTemp(Response));
		return true;
	}

	// TODO: Fire Once here
	TUniquePtr<FHttpServerResponse> Response = CreateSimpleResponse(true);
	OnComplete(MoveTemp(Response));
	return true;
}

bool UNoodlingGameplayRpcRegistrationComponent::HttpGetPlayerVitalsCommand(const FHttpServerRequest& Request,
	const FHttpResultCallback& OnComplete)
{
	ANoodlingPlayerController* NoodlingPlayerController = GetPlayerController();
	if (!NoodlingPlayerController)
	{
		TUniquePtr<FHttpServerResponse> Response = CreateSimpleResponse(false, TEXT("No player controller found"));
		OnComplete(MoveTemp(Response));
		return true;
	}
	APawn* PlayerPawn = NoodlingPlayerController->GetPawn();
	if (!PlayerPawn)
	{
		TUniquePtr<FHttpServerResponse> Response = CreateSimpleResponse(false, TEXT("Player pawn not found"));
		OnComplete(MoveTemp(Response));
		return true;
	}

	if (const ANoodlingPlayerState* NoodlingPlayerState = NoodlingPlayerController->GetNoodlingPlayerState(); !NoodlingPlayerState)
	{
		TUniquePtr<FHttpServerResponse> Response = CreateSimpleResponse(false, TEXT("Player state not found"));
		OnComplete(MoveTemp(Response));
		return true;
	}

	FString ResponseStr;
	const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&ResponseStr);
	TSharedPtr<FJsonObject> BodyObject = MakeShareable(new FJsonObject());
	JsonWriter->WriteObjectStart();
	// TODO: Uncomment after UNoodlingHealthComponent and UNoodlingInventoryManagerComponent are implemented.
	// if (UNoodlingHealthComponent* HealthComponent = UNoodlingHealthComponent::FindHealthComponent(PlayerPawn))
	// {
	// 	JsonWriter->WriteValue(TEXT("health"), FString::SanitizeFloat(HealthComponent->GetHealth()));
	// }
	// if (UNoodlingInventoryManagerComponent* InventoryComponent = NoodlingPlayerController->GetComponentByClass<UNoodlingInventoryManagerComponent>())
	// {
	// 	JsonWriter->WriteArrayStart(TEXT("inventory"));
	// 	for (UNoodlingInventoryItemInstance* ItemInstance : InventoryComponent->GetAllItems())
	// 	{
	// 		// TODO: Dump any relevant player info here.
	// 	}
	// 	JsonWriter->WriteArrayEnd();
	// }
	JsonWriter->WriteObjectEnd();
	JsonWriter->Close();
	TUniquePtr<FHttpServerResponse>Response = FHttpServerResponse::Create(ResponseStr, TEXT("application/json"));
	OnComplete(MoveTemp(Response));
	return true;
}


