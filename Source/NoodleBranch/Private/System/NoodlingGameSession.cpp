// Fill out your copyright notice in the Description page of Project Settings.


#include "System/NoodlingGameSession.h"

ANoodlingGameSession::ANoodlingGameSession(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool ANoodlingGameSession::ProcessAutoLogin()
{
	// This is actually handled in NoodlingGameMode::TryDedicatedServerLogin
	return true;
}

void ANoodlingGameSession::HandleMatchHasStarted()
{
	// No customization yet
	Super::HandleMatchHasStarted();
}

void ANoodlingGameSession::HandleMatchHasEnded()
{
	// No customization yet
	Super::HandleMatchHasEnded();
}
