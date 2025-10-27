// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SignificanceManager.h"
#include "NoodlingSignificanceManager.generated.h"

/*
 * This Requires:
 * [/Script/SignificanceManager.SignificanceManager]
 * SignificanceManagerClassName=/Script/NoodleBranch.NoodlingSignificanceManager
 * To be set in DefaultEngine.ini in order to be used.
 */
UCLASS()
class NOODLEBRANCH_API UNoodlingSignificanceManager : public USignificanceManager
{
	GENERATED_BODY()
};
