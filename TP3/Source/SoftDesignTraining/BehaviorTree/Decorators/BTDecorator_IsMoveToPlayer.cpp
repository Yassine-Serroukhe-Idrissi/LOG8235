// Fill out your copyright notice in the Description page of Project Settings.

#include "BTDecorator_IsMoveToPlayer.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include <SoftDesignTraining/AiAgentGroupManager.h>

bool UBTDecorator_IsMoveToPlayer::CalculateRawConditionValue(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory) const
{
    ASDTAIController *AIController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
    ASoftDesignTrainingCharacter *ControlledCharacter = Cast<ASoftDesignTrainingCharacter>(AIController->GetPawn());
    AiAgentGroupManager *aiAgentGroupManager = AiAgentGroupManager::GetInstance();
    if (!AIController)
    {
        return false;
    }

    ASDTAIController::PlayerInteractionBehavior playerBehavior = static_cast<ASDTAIController::PlayerInteractionBehavior>(OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Enum>(AIController->GetPlayerInteractionBehaviorKeyID()));

    if (playerBehavior != ASDTAIController::PlayerInteractionBehavior::PlayerInteractionBehavior_Chase)

        aiAgentGroupManager->UnregisterAIAgent(ControlledCharacter);
    return false;

    aiAgentGroupManager->RegisterAIAgent(ControlledCharacter);
    return true;
}
