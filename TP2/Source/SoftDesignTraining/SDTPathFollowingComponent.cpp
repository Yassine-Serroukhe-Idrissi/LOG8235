// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTPathFollowingComponent.h"
#include "SoftDesignTraining.h"
#include "SDTUtils.h"
#include "SDTAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "NavLinkCustomInterface.h"

#include "DrawDebugHelpers.h"
#include "SoftDesignTrainingPlayerController.h"

USDTPathFollowingComponent::USDTPathFollowingComponent(const FObjectInitializer& ObjectInitializer)
{

}

/**
* This function is called every frame while the AI is following a path.
* MoveSegmentStartIndex and MoveSegmentEndIndex specify where we are on the path point array.
*/
void USDTPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
    const TArray<FNavPathPoint>& points = Path->GetPathPoints();
    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];
    const FNavPathPoint& segmentEnd = points[MoveSegmentEndIndex];

    if (SDTUtils::HasJumpFlag(segmentStart))
    {
        // Update jump along path / nav link proxy
        jumProgress += DeltaTime;
    }
    else
    {
        // Update navigation along path (move along)
        Super::FollowPathSegment(DeltaTime);
    }
}

/**
* This function is called every time the AI has reached a new point on the path.
* If you need to do something at a given point in the path, this is the place.
*/
void USDTPathFollowingComponent::SetMoveSegment(int32 segmentStartIndex)
{
    Super::SetMoveSegment(segmentStartIndex);

    const TArray<FNavPathPoint>& points = Path->GetPathPoints();

    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];

    ASoftDesignTrainingPlayerController* PlayerController = Cast<ASoftDesignTrainingPlayerController>(GetOwner());

    UCharacterMovementComponent* MovementComponent;

    if (PlayerController != nullptr) {
        MovementComponent = PlayerController->GetCharacter()->GetCharacterMovement();
        if (SDTUtils::HasJumpFlag(segmentStart) && FNavMeshNodeFlags(segmentStart.Flags).IsNavLink()) {
            isJumping = true;
            MovementComponent->SetMovementMode(EMovementMode::MOVE_Falling);
            FVector NextLocation = points[MoveSegmentStartIndex + 1].Location;
            FVector JumpDirection = (NextLocation - segmentStart.Location).GetSafeNormal();
            FRotator JumpRotation = JumpDirection.Rotation();
            PlayerController->GetCharacter()->SetActorRotation(JumpRotation);
            FVector Launchspeed = FVector((NextLocation.X - segmentStart.Location.X) / 2.0f, (NextLocation.Y - segmentStart.Location.Y) / 2.0f, 1000.0f);
            MovementComponent->Launch(Launchspeed);
            jumProgress = 0.f;
        }
        else {
            isJumping = false;
            MovementComponent->SetMovementMode(EMovementMode::MOVE_Walking);
        }
    }
    else {
        ASDTAIController* aiController = Cast<ASDTAIController>(GetOwner());
        MovementComponent = aiController->GetCharacter()->GetCharacterMovement();
        isJumping = false;
        MovementComponent->SetMovementMode(EMovementMode::MOVE_Walking);
    }
    
}

