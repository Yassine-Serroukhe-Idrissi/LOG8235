// Fill out your copyright notice in the Description page of Project Settings.

#include "BTDecorator_IsMoveToCollectible.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include <SoftDesignTraining/SoftDesignTrainingCharacter.h>
#include <SoftDesignTraining/AiAgentGroupManager.h>

bool UBTDecorator_IsMoveToCollectible::CalculateRawConditionValue(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory) const
{
    ASDTAIController *AIController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());

    ASoftDesignTrainingCharacter *ControlledCharacter = Cast<ASoftDesignTrainingCharacter>(AIController->GetPawn());
    if (!AIController)
    {
        return false;
    }

    ASDTAIController::PlayerInteractionBehavior playerBehavior = static_cast<ASDTAIController::PlayerInteractionBehavior>(OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Enum>(AIController->GetPlayerInteractionBehaviorKeyID()));

    if (playerBehavior != ASDTAIController::PlayerInteractionBehavior::PlayerInteractionBehavior_Collect)
        return false;

    return true;
}
