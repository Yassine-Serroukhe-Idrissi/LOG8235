// Fill out your copyright notice in the Description page of Project Settings.

#include "BTService_UpdatePlayerInteract.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "SoftDesignTraining/SoftDesignTrainingCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "SoftDesignTraining/SoftDesignTraining.h"
#include "SoftDesignTraining/SoftDesignTrainingCharacter.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "SoftDEsignTraining/SDTAIController.h"
#include "SoftDesignTraining/SDTPathFollowingComponent.h"
#include "SoftDEsignTraining/SDTUtils.h"
#include "EngineUtils.h"

#define COLLISION_PLAYER ECollisionChannel::ECC_GameTraceChannel4
#define COLLISION_COLLECTIBLE ECollisionChannel::ECC_GameTraceChannel5

UBTService_UpdatePlayerInteract::UBTService_UpdatePlayerInteract()
{
    bNotifyTick = true; // Enable ticking for this service
}

void UBTService_UpdatePlayerInteract::TickNode(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory, float DeltaSeconds)
{

    ASDTAIController *AIController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        return; // Early exit if the controller is not valid
    }

    if (AIController->AtJumpSegment)
        return;

    APawn *selfPawn = AIController->GetPawn();
    if (!selfPawn)
    {
        return;
    }

    ACharacter *playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
    {
        return;
    }

    if (AIController->GetMoveStatus() == EPathFollowingStatus::Idle)
    {
        OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(AIController->GetTargetReachedKeyID(), true);
    }
}