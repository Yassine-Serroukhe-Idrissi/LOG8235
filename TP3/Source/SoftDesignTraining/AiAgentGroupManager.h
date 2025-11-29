#pragma once

#include "SoftDesignTrainingCharacter.h"
#include "CoreMinimal.h"
#include "TargetLKPInfo.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "GameFramework/Actor.h"


/**
 *
 */
class SOFTDESIGNTRAINING_API AiAgentGroupManager
{
public:
    static AiAgentGroupManager* GetInstance();
    static void Destroy();

    void RegisterAIAgent(ASoftDesignTrainingCharacter* aiAgent);
    void UnregisterAIAgent(ASoftDesignTrainingCharacter* aiAgent);
    void DrawDebugIndicators(const UWorld* world);
    void AssignEncirclementPositions(FVector PlayerLocation, UWorld* world);
    TargetLKPInfo GetLKPFromGroup(const FString& targetLabel, bool& targetFound);
    void Initialize(/*ASoftDesignTrainingCharacter* Player*/);
   

private:

    //SINGLETON
    AiAgentGroupManager();
    static AiAgentGroupManager* m_Instance;
    float EncirclementRadius = 1000;
    float currentRadius = EncirclementRadius;
    int iterationBeforeReseting;
    int currentIteration = 0;
    int rateOfDecrease = 10;
    TArray<ASoftDesignTrainingCharacter*> m_registeredAgents;

};
