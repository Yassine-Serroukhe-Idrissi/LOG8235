// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SoftDesignTrainingCharacter.h"
#include "SDTCollectible.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
// #include "UnrealMathUtility.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "SDTUtils.h"
#include "EngineUtils.h"
#include "AiAgentGroupManager.h"

ASDTAIController::ASDTAIController(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
    m_behaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    m_blackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    m_PlayerInteractionBehavior = PlayerInteractionBehavior_Collect;
}

void ASDTAIController::StartBehaviorTree(APawn *pawn)
{
    if (ASoftDesignTrainingCharacter *aiBaseCharacter = Cast<ASoftDesignTrainingCharacter>(pawn))
    {
        if (aiBaseCharacter->GetBehaviorTree())
        {
            m_behaviorTreeComponent->StartTree(*aiBaseCharacter->GetBehaviorTree());
        }
    }
}

void ASDTAIController::StopBehaviorTree(APawn *pawn)
{
    if (ASoftDesignTrainingCharacter *aiBaseCharacter = Cast<ASoftDesignTrainingCharacter>(pawn))
    {
        if (aiBaseCharacter->GetBehaviorTree())
        {
            m_behaviorTreeComponent->StopTree();
        }
    }
}

void ASDTAIController::OnPossess(APawn *pawn)
{
    Super::OnPossess(pawn);

    if (ASoftDesignTrainingCharacter *aiBaseCharacter = Cast<ASoftDesignTrainingCharacter>(pawn))
    {
        if (aiBaseCharacter->GetBehaviorTree())
        {
            m_blackboardComponent->InitializeBlackboard(*aiBaseCharacter->GetBehaviorTree()->BlackboardAsset);

            m_selfActorKeyID = m_blackboardComponent->GetKeyID("SelfActor");
            m_playerKeyID = m_blackboardComponent->GetKeyID("Player");
            m_targetReachedKeyID = m_blackboardComponent->GetKeyID("TargetReached");
            m_jumpTargetKeyID = m_blackboardComponent->GetKeyID("JumpTarget");
            m_obstacleAvoidanceRotationKeyID = m_blackboardComponent->GetKeyID("ObstacleAvoidanceRotation");
            m_playerInteractionBehaviorKeyID = m_blackboardComponent->GetKeyID("PlayerInteractionBehavior");
            m_shouldExecuteServiceKeyID = m_blackboardComponent->GetKeyID("ShouldExecuteService");

            // Set this agent in the BT
            m_blackboardComponent->SetValue<UBlackboardKeyType_Object>(m_blackboardComponent->GetKeyID("SelfActor"), pawn);

            m_blackboardComponent->SetValue<UBlackboardKeyType_Bool>(m_blackboardComponent->GetKeyID("TargetReached"), true);
            bool bTargetReached = m_blackboardComponent->GetValue<UBlackboardKeyType_Bool>(GetTargetReachedKeyID());
            UE_LOG(LogTemp, Log, TEXT("Blackboard Key 'targetReached' state: %s"), bTargetReached ? TEXT("True") : TEXT("False"));
            m_blackboardComponent->SetValue<UBlackboardKeyType_Enum>(m_blackboardComponent->GetKeyID("PlayerInteractionBehavior"), static_cast<uint8>(PlayerInteractionBehavior::PlayerInteractionBehavior_Collect));
            m_blackboardComponent->SetValue<UBlackboardKeyType_Bool>(GetShouldExecuteServiceKeyID(), true);

            /*m_blackboardComponent->SetValue<UBlackboardKeyType_Object>(m_blackboardComponent->GetKeyID("Player"), playerCharacter);*/
        }
    }
}

void ASDTAIController::PlayerInteractionLoSUpdate()
{
    ACharacter *playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    FHitResult losHit;
    GetWorld()->LineTraceSingleByObjectType(losHit, GetPawn()->GetActorLocation(), playerCharacter->GetActorLocation(), TraceObjectTypes);

    bool hasLosOnPlayer = false;

    if (losHit.GetComponent())
    {
        if (losHit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER)
        {
            hasLosOnPlayer = true;
        }
    }

    if (hasLosOnPlayer)
    {
        if (GetWorld()->GetTimerManager().IsTimerActive(m_PlayerInteractionNoLosTimer))
        {
            GetWorld()->GetTimerManager().ClearTimer(m_PlayerInteractionNoLosTimer);
            m_PlayerInteractionNoLosTimer.Invalidate();
            // DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "Got LoS", GetPawn(), FColor::Red, 5.f, false);
        }
    }
    else
    {
        if (!GetWorld()->GetTimerManager().IsTimerActive(m_PlayerInteractionNoLosTimer))
        {
            GetWorld()->GetTimerManager().SetTimer(m_PlayerInteractionNoLosTimer, this, &ASDTAIController::OnPlayerInteractionNoLosDone, 3.f, false);
            // DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "Lost LoS", GetPawn(), FColor::Red, 5.f, false);
        }
    }
}

void ASDTAIController::OnPlayerInteractionNoLosDone()
{
    GetWorld()->GetTimerManager().ClearTimer(m_PlayerInteractionNoLosTimer);
    // DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "TIMER DONE", GetPawn(), FColor::Red, 5.f, false);

    if (!AtJumpSegment)
    {
        AIStateInterrupted();
        // m_PlayerInteractionBehavior = PlayerInteractionBehavior_Collect;
        m_blackboardComponent->SetValue<UBlackboardKeyType_Enum>(GetPlayerInteractionBehaviorKeyID(), PlayerInteractionBehavior_Collect);
    }
}

void ASDTAIController::OnMoveToTarget()
{
    // m_ReachedTarget = false;
    m_blackboardComponent->SetValue<UBlackboardKeyType_Bool>(GetTargetReachedKeyID(), false);
}

void ASDTAIController::RotateTowards(const FVector &targetLocation)
{
    if (!targetLocation.IsZero())
    {
        FVector direction = targetLocation - GetPawn()->GetActorLocation();
        FRotator targetRotation = direction.Rotation();

        targetRotation.Yaw = FRotator::ClampAxis(targetRotation.Yaw);

        SetControlRotation(targetRotation);
    }
}

void ASDTAIController::SetActorLocation(const FVector &targetLocation)
{
    GetPawn()->SetActorLocation(targetLocation);
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult &Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    // m_ReachedTarget = true;
    m_blackboardComponent->SetValue<UBlackboardKeyType_Bool>(GetTargetReachedKeyID(), true);
}

void ASDTAIController::ShowNavigationPath()
{
    if (UPathFollowingComponent *pathFollowingComponent = GetPathFollowingComponent())
    {
        if (pathFollowingComponent->HasValidPath())
        {
            const FNavPathSharedPtr path = pathFollowingComponent->GetPath();
            TArray<FNavPathPoint> pathPoints = path->GetPathPoints();

            for (int i = 0; i < pathPoints.Num(); ++i)
            {
                // DrawDebugSphere(GetWorld(), pathPoints[i].Location, 10.f, 8, FColor::Yellow);

                if (i != 0)
                {
                    // DrawDebugLine(GetWorld(), pathPoints[i].Location, pathPoints[i - 1].Location, FColor::Yellow);
                }
            }
        }
    }
}

void ASDTAIController::UpdatePlayerInteraction(float deltaTime)
{
    // finish jump before updating AI state
    if (AtJumpSegment)
        return;

    APawn *selfPawn = GetPawn();
    if (!selfPawn)
        return;

    ACharacter *playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
    FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;

    TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    TArray<FHitResult> allDetectionHits;
    GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

    FHitResult detectionHit;
    GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);

    UpdatePlayerInteractionBehavior(detectionHit, deltaTime);

    if (GetMoveStatus() == EPathFollowingStatus::Idle)
    {
        m_ReachedTarget = true;
    }

    FString debugString = "";

    switch (m_PlayerInteractionBehavior)
    {
    case PlayerInteractionBehavior_Chase:
        debugString = "Chase";
        break;
    case PlayerInteractionBehavior_Flee:
        debugString = "Flee";
        break;
    case PlayerInteractionBehavior_Collect:
        debugString = "Collect";
        break;
    case PlayerInteractionBehavior_InvestigateLKP:
        debugString = "InvestigateLKP";
        break;
    }
    // AiAgentGroupManager::GetInstance()->DrawDebugIndicators(GetWorld());

     DrawDebugString(GetWorld(), FVector(0.f, 0.f, 5.f), debugString, GetPawn(), FColor::Orange, 0.f, false);

    // DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
}

bool ASDTAIController::HasLoSOnHit(const FHitResult &hit)
{
    if (!hit.GetComponent())
        return false;

    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));

    FVector hitDirection = hit.ImpactPoint - hit.TraceStart;
    hitDirection.Normalize();

    FHitResult losHit;
    FCollisionQueryParams queryParams = FCollisionQueryParams();
    queryParams.AddIgnoredActor(hit.GetActor());

    GetWorld()->LineTraceSingleByObjectType(losHit, hit.TraceStart, hit.ImpactPoint + hitDirection, TraceObjectTypes, queryParams);

    return losHit.GetActor() == nullptr;
}

void ASDTAIController::AIStateInterrupted()
{
    StopMovement();
    // m_ReachedTarget = true;
    m_blackboardComponent->SetValue<UBlackboardKeyType_Bool>(GetTargetReachedKeyID(), true);
}

ASDTAIController::PlayerInteractionBehavior ASDTAIController::GetCurrentPlayerInteractionBehavior(const FHitResult& hit)
{
    ACharacter* player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    ASoftDesignTrainingCharacter* character = Cast<ASoftDesignTrainingCharacter>(GetPawn());

    if (!player || !character)
        return PlayerInteractionBehavior_Collect;

    bool playerDetected = hit.GetComponent() &&
        hit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER;

    bool hasLoS = playerDetected && HasLoSOnHit(hit);

    PlayerInteractionBehavior current =
        (PlayerInteractionBehavior)m_blackboardComponent->GetValue<UBlackboardKeyType_Enum>(
            GetPlayerInteractionBehaviorKeyID()
        );

    if (current == PlayerInteractionBehavior_Chase && !hasLoS)
    {
        return PlayerInteractionBehavior_Chase;
    }

    if (!hasLoS)
    {
        return PlayerInteractionBehavior_Collect;
    }

    character->m_currentTargetLkpInfo.SetLKPPos(player->GetActorLocation());
    character->m_currentTargetLkpInfo.SetLKPState(TargetLKPInfo::ELKPState::LKPState_ValidByLOS);
    character->m_currentTargetLkpInfo.SetTargetLabel(player->GetActorLabel());
    character->m_currentTargetLkpInfo.SetLastUpdatedTimeStamp(
        UGameplayStatics::GetRealTimeSeconds(GetWorld())
    );

    return SDTUtils::IsPlayerPoweredUp(GetWorld())
        ? PlayerInteractionBehavior_Flee
        : PlayerInteractionBehavior_Chase;
}


void ASDTAIController::GetHightestPriorityDetectionHit(const TArray<FHitResult> &hits, FHitResult &outDetectionHit)
{

    for (const FHitResult &hit : hits)
    {
        if (UPrimitiveComponent *component = hit.GetComponent())
        {
            if (component->GetCollisionObjectType() == COLLISION_PLAYER)
            {
                // we can't get more important than the player
                outDetectionHit = hit;
                return;
            }
            else if (component->GetCollisionObjectType() == COLLISION_COLLECTIBLE)
            {
                outDetectionHit = hit;
            }
        }
    }
}

void ASDTAIController::UpdatePlayerInteractionBehavior(const FHitResult &detectionHit, float deltaTime)
{
    PlayerInteractionBehavior currentBehavior = GetCurrentPlayerInteractionBehavior(detectionHit);

    if (currentBehavior != m_PlayerInteractionBehavior)
    {
        m_PlayerInteractionBehavior = currentBehavior;
        AIStateInterrupted();
    }
}
