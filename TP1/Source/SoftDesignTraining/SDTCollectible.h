// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
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

    virtual void Tick(float deltaTime) override;
    virtual void BeginPlay() override;

    FVector initialPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
    USoundBase* m_CollectionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
    UParticleSystem* m_CollectionEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
    float m_SoundVolume = 1.0f;


protected:
    FTimerHandle m_CollectCooldownTimer;
private:

    void TriggerCollectionFeedback();
};
