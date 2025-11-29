// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MoveToBestFleeLocation.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "SoftDesignTraining/SDTFleeLocation.h"
#include "SoftDesignTraining/SoftDesignTraining.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "EngineUtils.h"
#include <SoftDesignTraining/SoftDesignTrainingCharacter.h>
#include <SoftDesignTraining/AiAgentGroupManager.h>

EBTNodeResult::Type UBTTask_MoveToBestFleeLocation::ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory)
{
    ASDTAIController *AIController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());

    ASoftDesignTrainingCharacter *ControlledCharacter = Cast<ASoftDesignTrainingCharacter>(AIController->GetPawn());

    AiAgentGroupManager *aiAgentGroupManager = AiAgentGroupManager::GetInstance();
    aiAgentGroupManager->UnregisterAIAgent(ControlledCharacter);
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    float bestLocationScore = 0.f;
    ASDTFleeLocation *bestFleeLocation = nullptr;

    ACharacter *playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return EBTNodeResult::Failed;

    for (TActorIterator<ASDTFleeLocation> actorIterator(GetWorld(), ASDTFleeLocation::StaticClass()); actorIterator; ++actorIterator)
    {
        ASDTFleeLocation *fleeLocation = Cast<ASDTFleeLocation>(*actorIterator);
        if (fleeLocation)
        {
            float distToFleeLocation = FVector::Dist(fleeLocation->GetActorLocation(), playerCharacter->GetActorLocation());
            APawn *pawn = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Object>(AIController->GetSelfActorKeyID()));
            FVector selfToPlayer = playerCharacter->GetActorLocation() - pawn->GetActorLocation();
            selfToPlayer.Normalize();

            FVector selfToFleeLocation = fleeLocation->GetActorLocation() - pawn->GetActorLocation();
            selfToFleeLocation.Normalize();

            float fleeLocationToPlayerAngle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(selfToPlayer, selfToFleeLocation)));
            float locationScore = distToFleeLocation + fleeLocationToPlayerAngle * 100.f;

            UE_LOG(LogTemp, Log, TEXT("ActorLocation: %s"), *pawn->GetActorLocation().ToString());
            if (bestFleeLocation)
            {
                UE_LOG(LogTemp, Log, TEXT("BestFleeLocation: %s"), *bestFleeLocation->GetActorLocation().ToString());
            }
            if (fleeLocation)
                UE_LOG(LogTemp, Log, TEXT("fleeLocation: %s"), *fleeLocation->GetActorLocation().ToString());

            if (locationScore > bestLocationScore)
            {
                bestLocationScore = locationScore;
                bestFleeLocation = fleeLocation;
            }

            /* DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), FString::SanitizeFloat(locationScore), fleeLocation, FColor::Red, 5.f, false);*/
        }
    }

    if (bestFleeLocation)
    {
        AIController->MoveToLocation(bestFleeLocation->GetActorLocation(), 0.5f, false, true, false, false, NULL, false);
        AIController->OnMoveToTarget();
    }

    return EBTNodeResult::Succeeded;
}