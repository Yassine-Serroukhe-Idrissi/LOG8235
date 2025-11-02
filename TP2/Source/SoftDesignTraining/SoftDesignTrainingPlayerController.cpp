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
    m_moveID = FAIRequestID::InvalidRequest;
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

    if (m_PathFollowingComponent)
    {
        m_PathFollowingComponent->OnRequestFinished.AddUObject(this, &ASoftDesignTrainingPlayerController::OnPathFinished);
    }
}

void ASoftDesignTrainingPlayerController::ClearPathDebug()
{
    if (UWorld* World = GetWorld())
    {
        FlushPersistentDebugLines(World);
    }
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
    if (!m_CanMoveCharacter)
        return;
    APawn* pawn = GetPawn();
    if (!pawn)
        return;

    FHitResult hit;
    if (!GetHitResultUnderCursor(ECC_Visibility, true, hit))
        return;

    UNavigationSystemV1* navSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!navSystem)
        return;

    FNavLocation target;
    if (!navSystem->ProjectPointToNavigation(hit.ImpactPoint, target))
        return;

    ClearPathDebug();

    if (UNavigationPath* path = navSystem->FindPathToLocationSynchronously(this, pawn->GetActorLocation(), target.Location))
    {
        const TArray<FVector>& points = path->PathPoints;
        for (int32 i = 1; i < points.Num(); ++i)
        {
            const FVector start = points[i - 1];
            const FVector end = points[i];
            DrawDebugLine(GetWorld(), start + FVector(0, 0, 10), end + FVector(0, 0, 10), FColor::Green, true, -1.f, 0, 5.f);
            DrawDebugSphere(GetWorld(), start + FVector(0, 0, 10), 10.f, 6, FColor::Blue, true);
            DrawDebugSphere(GetWorld(), end + FVector(0, 0, 10), 10.f, 6, FColor::Red, true);
        }

        m_moveID = m_PathFollowingComponent->RequestMove(FAIMoveRequest(target), path->GetPath());
    }
}

void ASoftDesignTrainingPlayerController::OnPathFinished(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    if (!m_moveID.IsValid() || RequestID != m_moveID)
        return;

    const bool reached = (Result.Code == EPathFollowingResult::Success);

    if (reached)
    {
        ClearPathDebug();
    }

    m_moveID = FAIRequestID::InvalidRequest;
}

void ASoftDesignTrainingPlayerController::Activate()
{
    APawn* pawn = GetPawn();
    if (pawn == nullptr)
    {
        return;
    }

    m_CanMoveCharacter = false;
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