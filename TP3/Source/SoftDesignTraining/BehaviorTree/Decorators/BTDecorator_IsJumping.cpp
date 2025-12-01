// Fill out your copyright notice in the Description page of Project Settings.

#include "BTDecorator_IsJumping.h"
#include "SoftDesignTraining/SDTAIController.h"

bool UBTDecorator_IsJumping::CalculateRawConditionValue(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory) const
{
    ASDTAIController *AIController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        return false;
    }

    // finish jump before updating AI state
    if (AIController->AtJumpSegment)
    {
        return false;
    }
    return true;
}
