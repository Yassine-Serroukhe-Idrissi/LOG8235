// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_IsReachedTarget.h"
#include "SoftDesignTraining/SoftDesignTraining.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "SoftDesignTraining/SoftDesignTrainingCharacter.h"

#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

EBTNodeResult::Type UBTTask_IsReachedTarget::ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory)
{
    if (ASDTAIController *aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        if (aiController->m_ReachedTarget)
        {
            return EBTNodeResult::Succeeded;
        }
    }

    return EBTNodeResult::Failed;
}
