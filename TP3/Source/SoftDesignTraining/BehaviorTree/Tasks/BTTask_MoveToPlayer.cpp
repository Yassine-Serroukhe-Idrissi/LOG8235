#include "BTTask_MoveToPlayer.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "SoftDesignTraining/SoftDesignTrainingCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "SoftDesignTraining/SoftDesignTraining.h"
#include "SoftDesignTraining/SoftDesignTrainingCharacter.h"
#include <SoftDesignTraining/AiAgentGroupManager.h>

EBTNodeResult::Type UBTTask_MoveToPlayer::ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory)
{
    // Get the AI controller
    ASDTAIController *AIController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    ACharacter *playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return EBTNodeResult::Failed;
    AiAgentGroupManager *aiAgentGroupManager = AiAgentGroupManager::GetInstance();
    UE_LOG(LogTemp, Log, TEXT("Move to player"));

    ASoftDesignTrainingCharacter *ControlledCharacter = Cast<ASoftDesignTrainingCharacter>(AIController->GetPawn());
    //DrawDebugSphere(
    //    GetWorld(),
    //    ControlledCharacter->GetActorLocation() + FVector(0.f, 0.f, 20.f), // Offset to place the sphere above the character
    //    10.0f,                                                             // Sphere radius
    //    5,                                                                 // Sphere segments
    //    FColor::Yellow, false, 1                                           // Sphere color
    //);
    aiAgentGroupManager->AssignEncirclementPositions(playerCharacter->GetActorLocation(), GetWorld());

    AIController->OnMoveToTarget();
    /*AIController->MoveToActor(playerCharacter, 0.5f, false, true, true, NULL, false); */

    // Task succeeded
    return EBTNodeResult::Succeeded;
}
