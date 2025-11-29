// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MoveToRandomCollectible.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "SoftDesignTraining/SoftDesignTrainingCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "SoftDesignTraining/SoftDesignTraining.h"
#include "SoftDesignTraining/SoftDesignTrainingCharacter.h"
#include "SoftDesignTraining/SDTCollectible.h"
#include <SoftDesignTraining/AiAgentGroupManager.h>

EBTNodeResult::Type UBTTask_MoveToRandomCollectible::ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory)
{
    ASDTAIController *AIController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
    ASoftDesignTrainingCharacter *ControlledCharacter = Cast<ASoftDesignTrainingCharacter>(AIController->GetPawn());
    AiAgentGroupManager *aiAgentGroupManager = AiAgentGroupManager::GetInstance();
    aiAgentGroupManager->UnregisterAIAgent(ControlledCharacter);
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }
    float closestSqrCollectibleDistance = 18446744073709551610.f;
    ASDTCollectible *closestCollectible = nullptr;

    TArray<AActor *> foundCollectibles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTCollectible::StaticClass(), foundCollectibles);

    while (foundCollectibles.Num() != 0)
    {
        int index = FMath::RandRange(0, foundCollectibles.Num() - 1);

        ASDTCollectible *collectibleActor = Cast<ASDTCollectible>(foundCollectibles[index]);
        if (!collectibleActor)
            return EBTNodeResult::Failed;

        if (!collectibleActor->IsOnCooldown())
        {
            AIController->MoveToLocation(foundCollectibles[index]->GetActorLocation(), 0.5f, false, true, true, false, NULL, false);
            AIController->OnMoveToTarget();
            return EBTNodeResult::Succeeded;
        }
        else
        {
            foundCollectibles.RemoveAt(index);
        }
    }
    return EBTNodeResult::Succeeded;
}
