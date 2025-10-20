// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NoodlingPawn.h"

ANoodlingPawn::ANoodlingPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// Called to bind functionality to input
void ANoodlingPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

