// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTPathFollowingComponent.h"
#include "SoftDesignTraining.h"
#include "SDTUtils.h"
#include "SDTAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "DrawDebugHelpers.h"

USDTPathFollowingComponent::USDTPathFollowingComponent(const FObjectInitializer& ObjectInitializer)
{

}

void USDTPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
    if (!Path.IsValid() || !NavMovementInterface.IsValid())
    {
        return;
    }

    const FVector CurrentLocation = NavMovementInterface->GetFeetLocation();
    const FVector CurrentTarget = GetCurrentTargetLocation();
    const TArray<FNavPathPoint>& points = Path->GetPathPoints();

    const FNavPathPoint& SegmentStart = points[MoveSegmentStartIndex];

    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), FString::SanitizeFloat(m_JumpProgressRatio), GetOwner()->GetParentActor(), FColor::Red, 0.f, false);

    if (SDTUtils::HasJumpFlag(SegmentStart))
    {
        ASDTAIController* controller = Cast<ASDTAIController>(GetOwner());
        if (controller)
        {
            if (controller->InAir)
            {
                m_JumpProgressRatio += DeltaTime;
                FVector JumpOffset = FVector(0.f, 0.f, 0.f);

                FVector nextLocation = FMath::Lerp(SegmentStart.Location, CurrentTarget, m_JumpProgressRatio);

                float jumpHeight = controller->JumpCurve->GetFloatValue(m_JumpProgressRatio) * controller->JumpApexHeight;
                nextLocation += FVector(0.f, 0.f, jumpHeight) + JumpOffset;

                const int32 LastSegmentStartIndex = points.Num() - 2;
                const bool bNotFollowingLastSegment = (MoveSegmentStartIndex < LastSegmentStartIndex);

                NavMovementInterface->RequestDirectMove((nextLocation - controller->GetPawn()->GetActorLocation()) * controller->JumpSpeed, bNotFollowingLastSegment);

                DrawDebugSphere(GetWorld(), nextLocation, 10.f, 8, FColor::Red, false, 5.f);
            }
            else
            {
                if (controller->Landing)
                {
                    ACharacter* character = Cast<ACharacter>(controller->GetPawn());
                    if (character)
                    {
                        UCharacterMovementComponent* charMoveComp = Cast<UCharacterMovementComponent>(character->GetMovementComponent());
                        if (charMoveComp)
                        {
                            charMoveComp->SetMovementMode(MOVE_Walking);
                        }
                    }
                }

                const int32 LastSegmentStartIndex = points.Num() - 2;
                const bool bNotFollowingLastSegment = (MoveSegmentStartIndex < LastSegmentStartIndex);
                NavMovementInterface->RequestDirectMove((CurrentTarget - controller->GetPawn()->GetActorLocation()), bNotFollowingLastSegment);
            }
        }
        
        return;
    }

    // set to false by default, we will set set this back to true if appropriate
    bIsDecelerating = false;
    
    const bool bAccelerationBased = NavMovementInterface->UseAccelerationForPathFollowing();
    if (bAccelerationBased)
    {
        CurrentMoveInput = (CurrentTarget - CurrentLocation).GetSafeNormal();

        if (MoveSegmentStartIndex >= DecelerationSegmentIndex)
        {
            const FVector PathEnd = Path->GetEndLocation();
            const float DistToEndSq = FVector::DistSquared(CurrentLocation, PathEnd);
            const bool bShouldDecelerate = DistToEndSq < FMath::Square(CachedBrakingDistance);
            if (bShouldDecelerate)
            {
                bIsDecelerating = true;

                const float SpeedPct = FMath::Clamp(FMath::Sqrt(DistToEndSq) / CachedBrakingDistance, 0.0f, 1.0f);
                CurrentMoveInput *= SpeedPct;
            }
        }

        PostProcessMove.ExecuteIfBound(this, CurrentMoveInput);
        NavMovementInterface->RequestPathMove(CurrentMoveInput);
    }
    else
    {
        FVector MoveVelocity = (CurrentTarget - CurrentLocation) / DeltaTime;

        const int32 LastSegmentStartIndex = points.Num() - 2;
        const bool bNotFollowingLastSegment = (MoveSegmentStartIndex < LastSegmentStartIndex);

        PostProcessMove.ExecuteIfBound(this, MoveVelocity);
        NavMovementInterface->RequestDirectMove(MoveVelocity, bNotFollowingLastSegment);
    }
}

void USDTPathFollowingComponent::SetMoveSegment(int32 SegmentStartIndex)
{
    Super::SetMoveSegment(SegmentStartIndex);

    const TArray<FNavPathPoint>& points = Path->GetPathPoints();

    const FNavPathPoint& SegmentStart = points[MoveSegmentStartIndex];

    m_JumpProgressRatio = 0.f;

    if (SDTUtils::HasJumpFlag(SegmentStart) && FNavMeshNodeFlags(SegmentStart.Flags).IsNavLink())
    {
        ASDTAIController* controller = Cast<ASDTAIController>(GetOwner());
        if (controller)
        {
            ACharacter* character = Cast<ACharacter>(controller->GetPawn());
            if (character)
            {
                UCharacterMovementComponent* charMoveComp = Cast<UCharacterMovementComponent>(character->GetMovementComponent());
                if (charMoveComp)
                {
                    controller->GetPawn()->bUseControllerRotationYaw = true;
                    controller->AtJumpSegment = true;

                    NavMovementInterface->StopMovementKeepPathing();

                    charMoveComp->bOrientRotationToMovement = false;
                    charMoveComp->SetMovementMode(MOVE_Flying);
                }
            }
        }
    }
    else
    {
        ASDTAIController* controller = Cast<ASDTAIController>(GetOwner());
        if (controller)
        {
            ACharacter* character = Cast<ACharacter>(controller->GetPawn());
            if (character)
            {
                UCharacterMovementComponent* charMoveComp = Cast<UCharacterMovementComponent>(character->GetMovementComponent());
                if (charMoveComp)
                {
                    controller->GetPawn()->bUseControllerRotationYaw = false;
                    controller->AtJumpSegment = false;
                    controller->Landing = false;

                    charMoveComp->bOrientRotationToMovement = true;
                    charMoveComp->SetMovementMode(MOVE_Walking);
                }
            }
        }
    }
}

