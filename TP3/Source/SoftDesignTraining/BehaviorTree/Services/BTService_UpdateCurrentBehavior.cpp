#include "BTService_UpdateCurrentBehavior.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "SoftDesignTraining/SoftDesignTrainingCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "SoftDesignTraining/SDTPathFollowingComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"

#define COLLISION_PLAYER ECollisionChannel::ECC_GameTraceChannel4

EBTNodeResult::Type UBTService_UpdateCurrentBehavior::ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory)
{
    ASDTAIController *AIController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    APawn *selfPawn = AIController->GetPawn();
    if (!selfPawn)
    {
        return EBTNodeResult::Failed;
    }

    // Retrieve player character directly
    ACharacter *playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
    {
        return EBTNodeResult::Failed;
    }

    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * AIController->m_DetectionCapsuleForwardStartingOffset;
    FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * AIController->m_DetectionCapsuleHalfLength * 2;

    TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    TArray<FHitResult> allDetectionHits;
    GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(AIController->m_DetectionCapsuleRadius));

    FHitResult detectionHit;
    AIController->GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);

    AIController->m_detectionHit = detectionHit;

    ASDTAIController::PlayerInteractionBehavior currentBehavior = AIController->GetCurrentPlayerInteractionBehavior(AIController->m_detectionHit);

    ASDTAIController::PlayerInteractionBehavior playerBehavior = static_cast<ASDTAIController::PlayerInteractionBehavior>(OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Enum>(AIController->GetPlayerInteractionBehaviorKeyID()));

    if (currentBehavior != playerBehavior)
    {
        OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Enum>(AIController->GetPlayerInteractionBehaviorKeyID(), currentBehavior);
        AIController->AIStateInterrupted();
    }

    if (AIController->GetMoveStatus() == EPathFollowingStatus::Idle)
    {
        OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(AIController->GetTargetReachedKeyID(), true);
    }

    return EBTNodeResult::Succeeded;
}
