// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SDTBaseAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SDTAIController.generated.h"

/**
 *
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public ASDTBaseAIController
{
    GENERATED_BODY()

public:
    ASDTAIController(const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get());
    enum PlayerInteractionBehavior
    {
        PlayerInteractionBehavior_Collect,
        PlayerInteractionBehavior_Chase,
        PlayerInteractionBehavior_Flee,
        PlayerInteractionBehavior_InvestigateLKP
    };

    FHitResult m_detectionHit;
    FVector m_detectionStartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleHalfLength = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleRadius = 250.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleForwardStartingOffset = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    UCurveFloat *JumpCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float JumpApexHeight = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float JumpSpeed = 1.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool AtJumpSegment = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool InAir = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool Landing = false;

    UBehaviorTreeComponent *GetBehaviorTreeComponent() const { return m_behaviorTreeComponent; }
    UBlackboardComponent *GetBlackBoardComponent() const { return m_blackboardComponent; }

    uint16 GetSelfActorKeyID() const { return m_selfActorKeyID; }
    uint16 GetPlayerKeyID() const { return m_playerKeyID; }
    uint16 GetJumpTargetKeyID() const { return m_jumpTargetKeyID; }
    uint16 GetObstacleAvoidanceRotationKeyID() const { return m_obstacleAvoidanceRotationKeyID; }
    uint16 GetTargetReachedKeyID() const { return m_targetReachedKeyID; }
    uint16 GetPlayerInteractionBehaviorKeyID() const { return m_playerInteractionBehaviorKeyID; }
    uint16 GetShouldExecuteServiceKeyID() const { return m_shouldExecuteServiceKeyID; }

    void StartBehaviorTree(APawn *pawn);
    void StopBehaviorTree(APawn *pawn);

protected:
    virtual void OnPossess(APawn *pawn);
    void UpdatePlayerInteractionBehavior(const FHitResult &detectionHit, float deltaTime);
    bool HasLoSOnHit(const FHitResult &hit);
    void MoveToRandomCollectible();
    void MoveToPlayer();
    void MoveToBestFleeLocation();
    void PlayerInteractionLoSUpdate();
    void OnPlayerInteractionNoLosDone();

public:
    virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult &Result) override;
    void RotateTowards(const FVector &targetLocation);
    void SetActorLocation(const FVector &targetLocation);
    void AIStateInterrupted();
    virtual void UpdatePlayerInteraction(float deltaTime) override;
    virtual void ShowNavigationPath() override;
    void GetHightestPriorityDetectionHit(const TArray<FHitResult> &hits, FHitResult &outDetectionHit);
    PlayerInteractionBehavior GetCurrentPlayerInteractionBehavior(const FHitResult &hit);
    void OnMoveToTarget();

private:
    UPROPERTY(transient)
    UBehaviorTreeComponent *m_behaviorTreeComponent;
    UPROPERTY(transient)
    UBlackboardComponent *m_blackboardComponent;

    uint16 m_selfActorKeyID;
    uint16 m_playerKeyID;
    uint16 m_targetReachedKeyID;
    uint16 m_jumpTargetKeyID;
    uint16 m_obstacleAvoidanceRotationKeyID;
    uint16 m_playerInteractionBehaviorKeyID;
    uint16 m_shouldExecuteServiceKeyID;

    virtual void GoToBestTarget(float deltaTime) override;
    bool m_NeedsUpdateNextFrame = false;

protected:
    FVector m_JumpTarget;
    FRotator m_ObstacleAvoidanceRotation;
    FTimerHandle m_PlayerInteractionNoLosTimer;
    PlayerInteractionBehavior m_PlayerInteractionBehavior;
};
