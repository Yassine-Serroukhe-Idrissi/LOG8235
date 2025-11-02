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

    if (SDTUtils::HasJumpFlag(Start))
    {
        jumpProgress += DeltaTime;
        UE_LOG(LogTemp, Warning, TEXT("in hasjumpflag"));
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

    UCharacterMovementComponent* MovementComponent = nullptr;

    if (PlayerController) {
        MovementComponent = PlayerController->GetCharacter()->GetCharacterMovement();

        if (SDTUtils::HasJumpFlag(segmentStart) && FNavMeshNodeFlags(segmentStart.Flags).IsNavLink())
        {
            // Handle starting jump
            isJumping = true;

            MovementComponent->SetMovementMode(EMovementMode::MOVE_Falling);

            FVector NextLocation = points[MoveSegmentStartIndex + 1].Location;
            FVector JumpDirection = (NextLocation - segmentStart.Location).GetSafeNormal();
            FRotator JumpRotation = JumpDirection.Rotation();
            PlayerController->GetCharacter()->SetActorRotation(JumpRotation);

            FVector LaunchSpeed(
                (NextLocation.X - segmentStart.Location.X) / 2.0f,
                (NextLocation.Y - segmentStart.Location.Y) / 2.0f,
                1000.0f
            );

            MovementComponent->Launch(LaunchSpeed);

            jumpProgress = 0.f;
        }
        else
        {
            // Handle normal segments
            isJumping = false;
            MovementComponent->SetMovementMode(EMovementMode::MOVE_Walking);
        }
    }
    else {
        // CAS POUR AI : UNIQUEMENT MARCHER PAS DE SAUT
        ASDTAIController* AIController = Cast<ASDTAIController>(GetOwner());
        MovementComponent = AIController->GetCharacter()->GetCharacterMovement();
        isJumping = false;
        MovementComponent->SetMovementMode(EMovementMode::MOVE_Walking);
    }

}
