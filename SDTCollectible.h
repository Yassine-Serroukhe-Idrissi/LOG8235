// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "PickupFeedbackComponent.h"   // ajout feedback
#include "SDTCollectible.generated.h"

/**
 *
 */
UCLASS()
class SOFTDESIGNTRAINING_API ASDTCollectible : public AStaticMeshActor
{
    GENERATED_BODY()
public:
    ASDTCollectible();

    void Collect();
    void OnCooldownDone();
    bool IsOnCooldown();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_CollectCooldownDuration = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    bool isMoveable = false;

    // --- Feedback : composant configurable dans l’éditeur ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Feedback")
    UPickupFeedbackComponent* FeedbackComponent;

    virtual void Tick(float deltaTime) override;
    virtual void BeginPlay() override;

    FVector initialPosition;

protected:
    FTimerHandle m_CollectCooldownTimer;
};

