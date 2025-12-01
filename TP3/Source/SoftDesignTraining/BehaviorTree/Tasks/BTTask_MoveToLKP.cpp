// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MoveToLKP.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "SoftDesignTraining/SoftDesignTrainingCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "SoftDesignTraining/SoftDesignTraining.h"
#include "SoftDesignTraining/SoftDesignTrainingCharacter.h"
#include <SoftDesignTraining/AiAgentGroupManager.h>

EBTNodeResult::Type UBTTask_MoveToLKP::ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory)
{
    // Get the AI controller
    ASDTAIController *AIController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());

    ASoftDesignTrainingCharacter *ControlledCharacter = Cast<ASoftDesignTrainingCharacter>(AIController->GetPawn());
    AiAgentGroupManager *aiAgentGroupManager = AiAgentGroupManager::GetInstance();
    aiAgentGroupManager->UnregisterAIAgent(ControlledCharacter);
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }
    APawn *pawn = AIController->GetPawn();
    ASoftDesignTrainingCharacter *character = Cast<ASoftDesignTrainingCharacter>(pawn);

    FVector lkpPos = character->m_currentTargetLkpInfo.GetLKPPos();

    if (lkpPos != FVector::ZeroVector)
    {
        AIController->MoveToLocation(lkpPos, 0.5f, false, true, true, false, NULL, false);
        AIController->OnMoveToTarget();
    }

    // Task succeeded
    return EBTNodeResult::Succeeded;
}