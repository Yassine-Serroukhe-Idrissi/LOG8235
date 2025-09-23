// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SDTAIController.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public AAIController
{
    GENERATED_BODY()
public:
    virtual void Tick(float deltaTime) override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI MOVEMENT")
    float MaxSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI MOVEMENT")
    float Acceleration = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI MOVEMENT")
    float RotationSpeed = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Wall Avoidance")
    float WallDetectionDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Wall Avoidance")
    float AvoidanceRotationSpeed = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Wall Avoidance")
    float MinSpeedWhenAvoiding = 0.2f;

private:
    FVector CurrentVelocity = FVector::ZeroVector;

    void CalculateVelocity(float deltaTime);
    void ApplyMovement(float deltaTime);
    void UpdateOrientation(float deltaTime);

    bool bIsAvoidingWall = false;
    FVector WallNormal = FVector::ZeroVector;
    float CurrentSpeedMultiplier = 1.0f;

    bool DetectWallAhead(float deltaTime);
    void AdjustSpeedForWallAvoidance(float deltaTime);
    void PerformWallAvoidanceRotation(float deltaTime);


};
