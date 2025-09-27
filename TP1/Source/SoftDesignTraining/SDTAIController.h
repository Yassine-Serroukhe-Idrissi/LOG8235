// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SDTCollectible.h"
#include "SDTAIController.generated.h"

/**
 *
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public AAIController
{
    GENERATED_BODY()

public:
    ASDTAIController();
    virtual void Tick(float deltaTime) override;

protected:
    // Question 1: Basic Movement Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float m_Acceleration = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float m_MaxVelocity = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float m_RotationSpeed = 180.0f;

    // Question 2: Wall Detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float m_WallDetectionDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float m_SlowdownDistance = 150.0f;

    // Question 3: Death Floor Detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float m_DeathFloorDetectionDistance = 180.0f;

    // Question 4: Pickup Detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float m_PickupDetectionDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float m_PickupCollectionDistance = 50.0f;

    // Question 6 & 7: Player Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Behavior")
    float m_PlayerDetectionDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Behavior")
    float m_PlayerPursuitSpeed = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Behavior")
    float m_FleeSpeed = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Behavior")
    float m_FleeDistance = 800.0f;

private:
    FVector m_CurrentVelocity = FVector::ZeroVector;
    FVector m_DesiredDirection = FVector::ForwardVector;
    AActor* m_CurrentPickupTarget = nullptr;

    // Player detection
    AActor* m_DetectedPlayer = nullptr;
    bool m_IsPlayerInRange = false;

    // Helper functions Questions 1-4
    bool DetectWalls(FVector& AvoidanceDirection);
    bool DetectDeathFloors(FVector& AvoidanceDirection);
    AActor* FindNearestPickup();
    bool IsPathClearToPickup(AActor* Pickup);
    FVector CalculateAvoidanceDirection(FVector HitNormal);

    // Question 6: Player pursuit functions
    bool DetectPlayer();
    bool IsPathClearToPlayer(AActor* Player);
    FVector CalculatePursuitDirection(const FVector& PlayerLocation);

    // Question 7: Flee behavior functions
    FVector CalculateFleeDirection(const FVector& PlayerLocation);
    bool IsPlayerTooClose();
    FVector FindBestFleeDirection(const FVector& BaseFleeDirection);
};
