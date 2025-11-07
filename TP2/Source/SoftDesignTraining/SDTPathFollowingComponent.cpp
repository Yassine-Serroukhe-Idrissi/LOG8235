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
    AController* Controller = GetOwner<AController>();
    if (!Controller) return;

    APawn* Pawn = Controller->GetPawn();
    if (!Pawn) return;

    ACharacter* Character = Cast<ACharacter>(Pawn);
    if (!Character) return;

    UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    if (!Movement) return;

    const TArray<FNavPathPoint>& Points = Path->GetPathPoints();
    const FNavPathPoint& Start = Points[MoveSegmentStartIndex];
    const FNavPathPoint& End = Points[MoveSegmentEndIndex];

    if (bPendingJumpLaunch)
    {
        Movement->bOrientRotationToMovement = false;

        const FRotator Curr = Character->GetActorRotation();
        const float Rate = JumpYawSpeedDegPerSec / 90.f;
        const FRotator NewRot = FMath::RInterpTo(Curr, JumpTargetRot, DeltaTime, Rate);
        Character->SetActorRotation(NewRot);

        if (NewRot.Equals(JumpTargetRot, 0.5f))
        {
            isJumping = true;
            Movement->SetMovementMode(EMovementMode::MOVE_Falling);
            Movement->Launch(CachedLaunchSpeed);

            bPendingJumpLaunch = false;

            Movement->bOrientRotationToMovement = true;
        }

        return;
    }

    if (SDTUtils::HasJumpFlag(Start))
    {
        jumpProgress += DeltaTime;
    }
    else
    {
        const FVector Current = Character->GetActorLocation();
        const FVector Target = End.Location;

        const float MaxSpeed = Movement->GetMaxSpeed();
        const FVector Dir = (Target - Current).GetSafeNormal2D();
        Movement->RequestDirectMove(Dir * MaxSpeed, false);
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
        if (SDTUtils::HasJumpFlag(segmentStart) && FNavMeshNodeFlags(segmentStart.Flags).IsNavLink())
        {
            const FVector NextLocation = points[MoveSegmentStartIndex + 1].Location;
            const FVector JumpDir = (NextLocation - segmentStart.Location).GetSafeNormal();

            const FRotator Desired = FRotationMatrix::MakeFromX(JumpDir).Rotator();
            JumpTargetRot = FRotator(0.f, Desired.Yaw, 0.f);

            CachedLaunchSpeed = FVector(
                (NextLocation.X - segmentStart.Location.X) / 2.f,
                (NextLocation.Y - segmentStart.Location.Y) / 2.f,
                1000.f
            );

            bPendingJumpLaunch = true;
            jumpProgress = 0.f;

            MovementComponent->SetMovementMode(EMovementMode::MOVE_Walking);
        }
        else
        {
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

