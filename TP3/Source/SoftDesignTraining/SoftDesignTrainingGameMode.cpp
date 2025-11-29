// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "SoftDesignTrainingGameMode.h"
#include "SoftDesignTraining.h"
#include "SoftDesignTrainingPlayerController.h"
#include "SoftDesignTrainingCharacter.h"
#include "AiAgentGroupManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

ASoftDesignTrainingGameMode::ASoftDesignTrainingGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ASoftDesignTrainingPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprint/BP_SDTMainCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ASoftDesignTrainingGameMode::StartPlay()
{
	Super::StartPlay();
	AiAgentGroupManager::GetInstance()->Initialize();
	// ASoftDesignTrainingCharacter* Player = Cast<ASoftDesignTrainingCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
	// if (Player)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("Player Found"));
	//	AiAgentGroupManager::GetInstance()->Initialize(Player); // Bind the delegate when the player is available
	// }
	// else
	//{

	//	UE_LOG(LogTemp, Log, TEXT("Player not Found"));
	//}

	GetWorld()->Exec(GetWorld(), TEXT("stat fps"));
}
