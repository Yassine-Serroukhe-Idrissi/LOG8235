// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "SoftDesignTrainingPlayerController.h"
#include "SoftDesignTraining.h"
#include "SoftDesignTrainingMainCharacter.h"

#include "DrawDebugHelpers.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "SDTBridge.h"
#include "SDTBoatOperator.h"
#include "Engine/OverlapResult.h"
#include <NavigationSystem.h>
#include <NavigationPath.h>
#include "SDTUtils.h"

ASoftDesignTrainingPlayerController::ASoftDesignTrainingPlayerController()
{
    // Make a path following component
    m_PathFollowingComponent = CreateDefaultSubobject<USDTPathFollowingComponent>(TEXT("PathFollowingComponent"));
}

void ASoftDesignTrainingPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Move camera
    InputComponent->BindAxis("MoveForward", this, &ASoftDesignTrainingPlayerController::MoveCameraForward);
    InputComponent->BindAxis("MoveRight", this, &ASoftDesignTrainingPlayerController::MoveCameraRight);

    // Zoom camera
    InputComponent->BindAxis("Zoom", this, &ASoftDesignTrainingPlayerController::ZoomCamera);

    // Move character on click
    InputComponent->BindAction("MoveCharacter", EInputEvent::IE_Released, this, &ASoftDesignTrainingPlayerController::MoveCharacter);
    
    InputComponent->BindAction("Activate", EInputEvent::IE_Pressed, this, &ASoftDesignTrainingPlayerController::Activate);
    InputComponent->BindAction("Activate", EInputEvent::IE_Released, this, &ASoftDesignTrainingPlayerController::Deactivate);
}

void ASoftDesignTrainingPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Always show the mouse
    bShowMouseCursor = true;

    // Allow character to be moved by default 
    m_CanMoveCharacter = true;

    // In case we are activating a bridge
    m_BridgeActivated = nullptr;

    // In case we are activating a boat operator
    m_BoatOperatorActivated = nullptr;
}

void ASoftDesignTrainingPlayerController::MoveCameraForward(float value)
{
    ASoftDesignTrainingMainCharacter* character = Cast<ASoftDesignTrainingMainCharacter>(GetPawn());

    if (character)
    {
        character->MoveCameraForward(value);
    }
}

void ASoftDesignTrainingPlayerController::MoveCameraRight(float value)
{
    ASoftDesignTrainingMainCharacter* character = Cast<ASoftDesignTrainingMainCharacter>(GetPawn());

    if (character)
    {
        character->MoveCameraRight(value);
    }
}

void ASoftDesignTrainingPlayerController::ZoomCamera(float axisValue)
{
    ASoftDesignTrainingMainCharacter* character = Cast<ASoftDesignTrainingMainCharacter>(GetPawn());

    if (character)
    {
        character->ZoomCamera(axisValue);
    }
}

void ASoftDesignTrainingPlayerController::MoveCharacter()
{
    // TODO : find the position of the mouse in the world 
    // And move the agent to this position IF possible
    // Validate you can move through m_CanMoveCharacter
   
    if (!m_CanMoveCharacter)
        return;

    FVector mouseLocation, mouseDirection;

    if (!DeprojectMousePositionToWorld(mouseLocation, mouseDirection))
        return;

    UE_LOG(LogTemp, Log, TEXT("Mouse World Location: X=%f, Y=%f, Z=%f"), mouseLocation.X, mouseLocation.Y, mouseLocation.Z);

    DrawDebugSphere(GetWorld(), mouseLocation, 0.1f, 12, FColor::Green, false, 1.0f);
    FVector Start = mouseLocation;

    const FVector startPosition = mouseLocation;
    const FVector endPosition = mouseLocation + (mouseDirection * 10000.0f);

    FCollisionQueryParams queryParams;
    queryParams.AddIgnoredActor(GetPawn());

    FHitResult hit;

    if (!GetWorld()->LineTraceSingleByChannel(hit, startPosition, endPosition, ECC_Visibility, queryParams))
        return;

    const FVector destination = hit.Location;
    const FVector targetBelow = destination - FVector(0.f, 0.f, 100.f);

    FVector navigationHit;
    UNavigationSystemV1::NavigationRaycast(GetWorld(), destination, targetBelow, navigationHit);

    const bool isValidDestination = (navigationHit - targetBelow).IsNearlyZero();

    if (!isValidDestination) {
        GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Red, TEXT("INVALID DESTINATION USED"));
        return;
    }

    UNavigationPath* navigationPath = UNavigationSystemV1::FindPathToLocationSynchronously(
        this,
        GetPawn()->GetActorLocation(),
        destination
    );

    if (!navigationPath || !navigationPath->GetPath().IsValid() || navigationPath->GetPath()->IsPartial() || navigationPath->GetPath()->GetPathPoints().Num() == 0) {
        GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Red, TEXT("Destination Utiliser Invalide"));
        return;
    }


    for (int32 i = 0; i < navigationPath->PathPoints.Num() - 1; i++) {
        const FVector start = navigationPath->PathPoints[i];
        const FVector end = navigationPath->PathPoints[i + 1];

        DrawDebugLine(GetWorld(), start, end, FColor::Green, false, 100.f, 0, 2.f);
        DrawDebugSphere(GetWorld(), start, 10.f, 12, FColor::Blue, false, 100.f);
        DrawDebugSphere(GetWorld(), end, 10.f, 12, FColor::Red, false, 100.f);

        if (SDTUtils::HasJumpFlag(navigationPath->PathPoints[i])) {
            ASoftDesignTrainingMainCharacter* character = Cast<ASoftDesignTrainingMainCharacter>(GetPawn());
            if (character) {
                character->Jump();
            }
            return;
        }
    }

    if (m_PathFollowingComponent && m_PathFollowingComponent->GetStatus() != EPathFollowingStatus::Idle) {
        m_PathFollowingComponent->AbortMove(
            *this,
            FPathFollowingResultFlags::ForcedScript | FPathFollowingResultFlags::NewRequest, FAIRequestID::CurrentRequest,
            EPathFollowingVelocityMode::Keep
        );
    }
    FAIMoveRequest moveRequest(destination);
    m_PathFollowingComponent->RequestMove(moveRequest, navigationPath->GetPath());
}

void ASoftDesignTrainingPlayerController::Activate()
{
    APawn* pawn = GetPawn();
    if (pawn == nullptr)
    {
        return;
    }

    m_CanMoveCharacter = false;
    // TODO : Mouvement of the agent should be stopped !!
    m_PathFollowingComponent->AbortMove(
        *this,
        FPathFollowingResultFlags::ForcedScript | FPathFollowingResultFlags::NewRequest, FAIRequestID::CurrentRequest,
        EPathFollowingVelocityMode::Keep
    );
    // Make an overlap to find what is near us to activate it
    TArray<FOverlapResult> results;
    GetWorld()->OverlapMultiByChannel(results, pawn->GetActorLocation(), pawn->GetActorRotation().Quaternion(), ECollisionChannel::ECC_WorldDynamic, FCollisionShape::MakeSphere(200.f));

    for (FOverlapResult& result : results)
    {
        AActor* actor = result.GetActor();
        ASDTBridge* bridge = Cast<ASDTBridge>(actor);
        if (bridge)
        {
            bridge->Activate();
            m_BridgeActivated = bridge;
            break;
        }

        ASDTBoatOperator* boatOperator = Cast<ASDTBoatOperator>(actor);
        if (boatOperator)
        {
            boatOperator->Activate();
            m_BoatOperatorActivated = boatOperator;
            break;
        }
    }
}

void ASoftDesignTrainingPlayerController::Deactivate()
{
    m_CanMoveCharacter = true;

    if (m_BridgeActivated)
    {
        m_BridgeActivated->Deactivate();
    }
    if (m_BoatOperatorActivated)
    {
        m_BoatOperatorActivated->Deactivate();
    }
}