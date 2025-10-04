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
    FVector CurrentVelocity;
    FVector LastAvoidanceDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Acceleration = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FloorDetectionDistance = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerDetectionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RotationSpeed = 15.f;

    virtual void Tick(float deltaTime) override;
    
private:
    void MovePawn(float deltaTime);
    void AvoidObstacles(float deltaTime);
    void ChaseCollectible(float deltaTime);
    void ChaseOrFleePlayer(float deltaTime);
};
