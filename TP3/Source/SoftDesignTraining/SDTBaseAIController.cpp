// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTBaseAIController.h"
#include "SoftDesignTraining.h"
#include "AiAgentGroupManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"

ASDTBaseAIController::ASDTBaseAIController(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    m_ReachedTarget = true;
}

void ASDTBaseAIController::Tick(float deltaTime)
{
    Super::Tick(deltaTime);
    TickRate();
}

void ASDTBaseAIController::TickRate()
{
    if (!GetPawn() || !GetWorld())
        return;

    APlayerController *PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController is on vacation."));
        return;
    }

    FVector PawnLocation = GetPawn()->GetActorLocation();
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();

    float Distance = FVector::Dist(PawnLocation, PlayerLocation);

    if (Distance < 5.0f)
    {
        SetActorTickInterval(0.1f);
    }
    else if (Distance < 10.0f)
    {
        SetActorTickInterval(0.5f);
    }
    else
    {
        SetActorTickInterval(2.0f);
    }
}