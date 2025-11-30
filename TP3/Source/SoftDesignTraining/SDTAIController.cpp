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

void ASDTAIController::GoToBestTarget(float deltaTime)
{
    ASoftDesignTrainingCharacter *ControlledCharacter = Cast<ASoftDesignTrainingCharacter>(GetPawn());
    switch (m_PlayerInteractionBehavior)
    {
    case PlayerInteractionBehavior_Collect:

        UE_LOG(LogTemp, Log, TEXT("Unregistered"));
        AiAgentGroupManager::GetInstance()->UnregisterAIAgent(ControlledCharacter);
        MoveToRandomCollectible();

        break;

    case PlayerInteractionBehavior_Chase:

        UE_LOG(LogTemp, Log, TEXT("Registered"));
        AiAgentGroupManager::GetInstance()->RegisterAIAgent(ControlledCharacter);
        // AiAgentGroupManager::GetInstance()->DrawDebugIndicators(GetWorld());
        MoveToPlayer();

        break;

    case PlayerInteractionBehavior_Flee:

        UE_LOG(LogTemp, Log, TEXT("Unregistered"));
        AiAgentGroupManager::GetInstance()->UnregisterAIAgent(ControlledCharacter);
        MoveToBestFleeLocation();

        break;
    }
}

void ASDTAIController::MoveToRandomCollectible()
{
    float closestSqrCollectibleDistance = 18446744073709551610.f;
    ASDTCollectible *closestCollectible = nullptr;

    TArray<AActor *> foundCollectibles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTCollectible::StaticClass(), foundCollectibles);

    while (foundCollectibles.Num() != 0)
    {
        int index = FMath::RandRange(0, foundCollectibles.Num() - 1);

        ASDTCollectible *collectibleActor = Cast<ASDTCollectible>(foundCollectibles[index]);
        if (!collectibleActor)
            return;

        if (!collectibleActor->IsOnCooldown())
        {
            MoveToLocation(foundCollectibles[index]->GetActorLocation(), 0.5f, false, true, true, false, NULL, false);
            OnMoveToTarget();
            return;
        }
        else
        {
            foundCollectibles.RemoveAt(index);
        }
    }
}

void ASDTAIController::MoveToPlayer()
{
    ACharacter *playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    // MoveToActor(playerCharacter, 0.5f, false, true, true, NULL, false);
    // OnMoveToTarget();
    // AiAgentGroupManager::GetInstance()->AssignEncirclementPositions(playerCharacter->GetActorLocation(),GetWorld());
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

void ASDTAIController::MoveToBestFleeLocation()
{
    float bestLocationScore = 0.f;
    ASDTFleeLocation *bestFleeLocation = nullptr;

    ACharacter *playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    for (TActorIterator<ASDTFleeLocation> actorIterator(GetWorld(), ASDTFleeLocation::StaticClass()); actorIterator; ++actorIterator)
    {
        ASDTFleeLocation *fleeLocation = Cast<ASDTFleeLocation>(*actorIterator);
        if (fleeLocation)
        {
            float distToFleeLocation = FVector::Dist(fleeLocation->GetActorLocation(), playerCharacter->GetActorLocation());

            FVector selfToPlayer = playerCharacter->GetActorLocation() - GetPawn()->GetActorLocation();
            selfToPlayer.Normalize();

            FVector selfToFleeLocation = fleeLocation->GetActorLocation() - GetPawn()->GetActorLocation();
            selfToFleeLocation.Normalize();

            float fleeLocationToPlayerAngle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(selfToPlayer, selfToFleeLocation)));
            float locationScore = distToFleeLocation + fleeLocationToPlayerAngle * 100.f;

            if (locationScore > bestLocationScore)
            {
                bestLocationScore = locationScore;
                bestFleeLocation = fleeLocation;
            }

            // DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), FString::SanitizeFloat(locationScore), fleeLocation, FColor::Red, 5.f, false);
        }
    }

    if (bestFleeLocation)
    {
        MoveToLocation(bestFleeLocation->GetActorLocation(), 0.5f, false, true, false, false, NULL, false);
        OnMoveToTarget();
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

ASDTAIController::PlayerInteractionBehavior ASDTAIController::GetCurrentPlayerInteractionBehavior(const FHitResult &hit)
{
    float currentElapsedTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());

    APawn *selfPawn = GetPawn();
    ASoftDesignTrainingCharacter *character = Cast<ASoftDesignTrainingCharacter>(selfPawn);

    ACharacter *playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    PlayerInteractionBehavior playerInteractionBehavior = static_cast<PlayerInteractionBehavior>(m_blackboardComponent->GetValue<UBlackboardKeyType_Enum>(GetPlayerInteractionBehaviorKeyID()));

    FVector lkpPos_1 = character->m_currentTargetLkpInfo.GetLKPPos();

    // TODO: make a service to show that
    /*DrawDebugSphere(GetWorld(), lkpPos_1, 30.0f, 32, FColor::Purple);
    DrawDebugSphere(GetWorld(), character->GetActorLocation() + FVector(0.f, 0.f, 100.f), 15.0f, 32, FColor::Purple);*/
    if (playerInteractionBehavior == PlayerInteractionBehavior_Collect)
    {
        // Check if group knows where player might be
        bool targetFoundByGroup = false;
        TargetLKPInfo targetInfoFromGroup = AiAgentGroupManager::GetInstance()->GetLKPFromGroup(playerCharacter->GetActorLabel(), targetFoundByGroup);
        // Check if group knows where player might be  [Exer on LKP sharing week 11]
        if (targetFoundByGroup)
        {
            character->m_currentTargetLkpInfo = targetInfoFromGroup;
            return PlayerInteractionBehavior_InvestigateLKP;
        }

        if (!hit.GetComponent())
            return PlayerInteractionBehavior_Collect;

        if (hit.GetComponent()->GetCollisionObjectType() != COLLISION_PLAYER)
            return PlayerInteractionBehavior_Collect;

        if (!HasLoSOnHit(hit))
            return PlayerInteractionBehavior_Collect;

        character->m_currentTargetLkpInfo.SetLKPPos(playerCharacter->GetActorLocation());
        character->m_currentTargetLkpInfo.SetLKPState(TargetLKPInfo::ELKPState::LKPState_ValidByLOS);
        character->m_currentTargetLkpInfo.SetTargetLabel(playerCharacter->GetActorLabel());
        character->m_currentTargetLkpInfo.SetLastUpdatedTimeStamp(currentElapsedTime);
        return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
    }
    else if (playerInteractionBehavior == PlayerInteractionBehavior_Chase)
    {
        if (hit.GetComponent())
        {
            if ((hit.GetComponent()->GetCollisionObjectType() != COLLISION_PLAYER) ||
                (hit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER && !HasLoSOnHit(hit)))
            {
                bool canAgentInvestigate = character->m_currentTargetLkpInfo.GetLKPState() == TargetLKPInfo::ELKPState::LKPState_ValidByLOS;
                if (canAgentInvestigate)
                {
                    // Set Lkp and investigation
                    character->m_currentTargetLkpInfo.SetLKPState(TargetLKPInfo::ELKPState::LKPState_Valid);
                    character->m_currentTargetLkpInfo.SetLastUpdatedTimeStamp(currentElapsedTime);
                    return PlayerInteractionBehavior_InvestigateLKP;
                }
                else
                {
                    // Check if group knows where player might be
                    bool targetFoundByGroup = false;
                    TargetLKPInfo targetInfoFromGroup = AiAgentGroupManager::GetInstance()->GetLKPFromGroup(playerCharacter->GetActorLabel(), targetFoundByGroup);
                    // Check if group knows where player might be  [Exer on LKP sharing week 11]
                    if (targetFoundByGroup)
                    {
                        character->m_currentTargetLkpInfo = targetInfoFromGroup;
                        return PlayerInteractionBehavior_InvestigateLKP;
                    }
                }
            }
            PlayerInteractionLoSUpdate();
            character->m_currentTargetLkpInfo.SetLKPPos(playerCharacter->GetActorLocation());
            character->m_currentTargetLkpInfo.SetLKPState(TargetLKPInfo::ELKPState::LKPState_ValidByLOS);
            character->m_currentTargetLkpInfo.SetTargetLabel(playerCharacter->GetActorLabel());
            character->m_currentTargetLkpInfo.SetLastUpdatedTimeStamp(currentElapsedTime);
            return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
        }
        else
        {
            PlayerInteractionLoSUpdate();
            character->m_currentTargetLkpInfo.SetLKPPos(playerCharacter->GetActorLocation());
            character->m_currentTargetLkpInfo.SetLKPState(TargetLKPInfo::ELKPState::LKPState_ValidByLOS);
            character->m_currentTargetLkpInfo.SetTargetLabel(playerCharacter->GetActorLabel());
            character->m_currentTargetLkpInfo.SetLastUpdatedTimeStamp(currentElapsedTime);
            return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
        }
        // return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
    }
    else if (playerInteractionBehavior == PlayerInteractionBehavior_InvestigateLKP)
    {

        bool targetFound = false;
        character->m_currentTargetLkpInfo = AiAgentGroupManager::GetInstance()->GetLKPFromGroup(playerCharacter->GetActorLabel(), targetFound);

        TargetLKPInfo::ELKPState currentInvestigatedLKPState = character->m_currentTargetLkpInfo.GetLKPState();
        currentInvestigatedLKPState = character->m_currentTargetLkpInfo.GetLKPState();

        FVector lkpPos = character->m_currentTargetLkpInfo.GetLKPPos();

        // TODO: make a service to show that
        /* DrawDebugSphere(GetWorld(), lkpPos, 30.0f, 32, FColor::Purple);
         DrawDebugSphere(GetWorld(), character->GetActorLocation() + FVector(0.f, 0.f, 100.f), 15.0f, 32, FColor::Purple);*/

        if (lkpPos != FVector::ZeroVector)
        {
            // return PlayerInteractionBehavior_InvestigateLKP;
        }

        if ((character->GetActorLocation() - lkpPos).Size2D() < 50.f)
        {
            character->m_currentTargetLkpInfo.SetLKPState(TargetLKPInfo::ELKPState::LKPState_Invalid);
            character->m_currentTargetLkpInfo.SetLastUpdatedTimeStamp(currentElapsedTime);
            return PlayerInteractionBehavior_Collect;
        }
        else if (TargetLKPInfo::ELKPState::LKPState_Invalid == currentInvestigatedLKPState)
        {
            return PlayerInteractionBehavior_Collect;
        }
        else if (hit.GetComponent())
        {
            if (hit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER && HasLoSOnHit(hit))
            {
                character->m_currentTargetLkpInfo.SetLKPPos(playerCharacter->GetActorLocation());
                character->m_currentTargetLkpInfo.SetLKPState(TargetLKPInfo::ELKPState::LKPState_ValidByLOS);
                character->m_currentTargetLkpInfo.SetTargetLabel(playerCharacter->GetActorLabel());
                character->m_currentTargetLkpInfo.SetLastUpdatedTimeStamp(currentElapsedTime);
                return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
            }
        }
        else if (!lkpPos.Equals(FVector::ZeroVector, KINDA_SMALL_NUMBER))
        {
            return PlayerInteractionBehavior_InvestigateLKP;
        }
        return PlayerInteractionBehavior_InvestigateLKP;
    }
    else
    {
        PlayerInteractionLoSUpdate();

        return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
    }
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
