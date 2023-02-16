// Copyright Epic Games, Inc. All Rights Reserved.

#include "TestWeaponProjectileGameMode.h"
#include "TestWeaponProjectileCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATestWeaponProjectileGameMode::ATestWeaponProjectileGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
