// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "TargetLKPInfo.h"
#include "SoftDesignTrainingCharacter.generated.h"

UCLASS()
class ASoftDesignTrainingCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASoftDesignTrainingCharacter();

    TargetLKPInfo m_currentTargetLkpInfo;
    bool m_isInvestigatingLKP;

    UBehaviorTree *GetBehaviorTree() const { return m_aiBehaviorTree; }

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
    virtual void OnCollectPowerUp() {};

    /* TMulticastDelegate<void()> OnPlayerDeath;*/
    void Die();
    const TargetLKPInfo &GetCurrentTargetLKPInfo() const { return m_currentTargetLkpInfo; }

protected:
    UFUNCTION()
    virtual void OnBeginOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

    UPROPERTY(EditAnywhere, category = Behavior)
    UBehaviorTree *m_aiBehaviorTree;

    FVector m_StartingPosition;
};
