// Fill out your copyright notice in the Description page of Project Settings.

#include "BTService_DrawDetectionCapsule.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"

UBTService_DrawDetectionCapsule::UBTService_DrawDetectionCapsule()
{
    bNotifyTick = true; // Enable ticking for this service
}

void UBTService_DrawDetectionCapsule::TickNode(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory, float DeltaSeconds)
{
    ASDTAIController *AIController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        return;
    }

    APawn *selfPawn = AIController->GetPawn();
    if (!selfPawn)
    {
        return;
    }

    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * AIController->m_DetectionCapsuleForwardStartingOffset;

    FString debugString = "";
    uint8 enumValue = static_cast<uint8>(OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Enum>(AIController->GetPlayerInteractionBehaviorKeyID()));
    ASDTAIController::PlayerInteractionBehavior playerInteractionBehavior = static_cast<ASDTAIController::PlayerInteractionBehavior>(enumValue);

    switch (playerInteractionBehavior)
    {
    case ASDTAIController::PlayerInteractionBehavior::PlayerInteractionBehavior_Chase:
        debugString = "Chase";
        break;
    case ASDTAIController::PlayerInteractionBehavior::PlayerInteractionBehavior_Flee:
        debugString = "Flee";
        break;
    case ASDTAIController::PlayerInteractionBehavior::PlayerInteractionBehavior_Collect:
        debugString = "Collect";
        break;
    case ASDTAIController::PlayerInteractionBehavior_InvestigateLKP:
        debugString = "InvestigateLKP";
        break;
    }

    /* DrawDebugString(GetWorld(), FVector(0.f, 0.f, 5.f), debugString, selfPawn, FColor::Orange, 0.f, false);
     DrawDebugCapsule(GetWorld(), detectionStartLocation + AIController->m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(),
         AIController->m_DetectionCapsuleHalfLength, AIController->m_DetectionCapsuleRadius,
         selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);*/
}