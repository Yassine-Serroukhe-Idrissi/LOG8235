#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SDTBaseAIController.generated.h"


UCLASS()
class SOFTDESIGNTRAINING_API ASDTBaseAIController : public AAIController
{
    GENERATED_BODY()

public:
    ASDTBaseAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    virtual void Tick(float deltaTime) override;
    bool m_ReachedTarget;
    void UpdateTickRate();

    bool IsVisibleInCameraView();

    static float TotalTimeBudgetUsed;
    static const float MaxTimeBudgetPerFrame;
    static int32 AgentsUpdatedThisFrame;

protected:
    virtual void RotationUpdate(float deltaTime) {};
    virtual void ImpulseToDirection(float deltaTime) {};

private:
    virtual void GoToBestTarget(float deltaTime) {};
    virtual void UpdatePlayerInteraction(float deltaTime) {};
    virtual void ShowNavigationPath() {};

    bool bWasVisibleLastFrame;
    bool bShouldTickThisFrame;
};