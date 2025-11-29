// Fill out your copyright notice in the Description page of Project Settings.

#include "AiAgentGroupManager.h"
#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "NavigationSystem.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "CoreMinimal.h"


AiAgentGroupManager* AiAgentGroupManager::m_Instance;

AiAgentGroupManager::AiAgentGroupManager()
{
}
void AiAgentGroupManager::Initialize(/*ASoftDesignTrainingCharacter* Player*/)
{
    EncirclementRadius = 400;
    currentRadius = EncirclementRadius;
    rateOfDecrease = 10;
    currentIteration = 0;
    iterationBeforeReseting = EncirclementRadius / rateOfDecrease + (EncirclementRadius / rateOfDecrease) * 0.5;
    //Tentative d'utiliser des event pour reset le radius lorsque le main perso meurt mais en vain
   /* if (Player)
    {

        UE_LOG(LogTemp, Log, TEXT("Player Initialized"));
        Player->OnPlayerDeath.AddLambda([this]()
            {
                currentRadius = EncirclementRadius;
                UE_LOG(LogTemp, Log, TEXT("Player died. Resetting radius."));
            });
    }*/
}

AiAgentGroupManager* AiAgentGroupManager::GetInstance()
{
    if (!m_Instance)
    {
        m_Instance = new AiAgentGroupManager();
    }

    return m_Instance;
}


void AiAgentGroupManager::AssignEncirclementPositions(FVector PlayerLocation, UWorld* world)
{
    UE_LOG(LogTemp, Log, TEXT("cURRENT rADIUS: %.2f"), currentRadius);
    int AgentCount = m_registeredAgents.Num();
    UE_LOG(LogTemp, Log, TEXT("Number Registerd: %d"), AgentCount);

    //if (AgentCount == 0) return;

    TArray<FVector> TargetPositions;

    // G�n�rer les positions autour du joueur
    float AngleStep = 360.0f / AgentCount;
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(world);
    float TimeSpent = 0.0f;
    float timeBudget = 0.002f;
    for (int i = 0; i < AgentCount; i++)
    {
        if (TimeSpent >= timeBudget)
        {
            return;
        }
        float StartTime = FPlatformTime::Seconds();
        float Angle = FMath::DegreesToRadians(i * AngleStep);
        FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0) * currentRadius;
        FVector CandidatePosition = PlayerLocation + Offset;

        // Check if the position is navigable
        FNavLocation NavLocation;
        bool IsNavigable = NavSys && NavSys->ProjectPointToNavigation(CandidatePosition, NavLocation);

        if (IsNavigable)
        {
            TargetPositions.Add(NavLocation.Location); //encercle
        }
        else
        {
            TargetPositions.Add(PlayerLocation); //sinon fonce sur le joueur
    
        }
        float UpdateTime = FPlatformTime::Seconds() - StartTime;
        TimeSpent += UpdateTime;    
    }


    // V�rifier qu'il y a assez de positions valides
   /* if (TargetPositions.Num() < AgentCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("Pas assez de positions valides pour tous les agents !"));
        return;
    }*/

    // Affecter chaque agent � la position la plus proche
    TArray<bool> PositionAssigned;
    PositionAssigned.Init(false, TargetPositions.Num());

    for (ASoftDesignTrainingCharacter* Agent : m_registeredAgents)
    {
        if (!Agent) continue;

        float MinDistance = FLT_MAX;
        int ClosestIndex = -1;

        FVector AgentLocation = Agent->GetActorLocation();

        // Trouver la position la plus proche non assign�e
        for (int i = 0; i < TargetPositions.Num(); i++)
        {
            if (!PositionAssigned[i])
            {
                float Distance = FVector::Dist(AgentLocation, TargetPositions[i]);
                if (Distance < MinDistance)
                {
                    MinDistance = Distance;
                    ClosestIndex = i;
                }
            }
        }

        // Affecter l'agent � la position trouv�e
        if (ClosestIndex != -1)
        {
            FVector AssignedPosition = TargetPositions[ClosestIndex];
            PositionAssigned[ClosestIndex] = true;

            ASDTAIController* AIController = Cast<ASDTAIController>(Agent->GetController());
            if (AIController)
            {
                // D�placer l'agent vers la position assign�e
                AIController->MoveToLocation(AssignedPosition, 0.5f, true, true, true, true, nullptr, true);

                //// D�bogage : Ligne vers la position assign�e
                //DrawDebugLine(
                //    world,
                //    AgentLocation,
                //    AssignedPosition,
                //    FColor::Blue,
                //    false,
                //    5.0f,
                //    0,
                //    2.0f
                //);

                // D�bogage : Sph�re sur la position assign�e
                DrawDebugCircle(
                    world,AssignedPosition,5,6, FColor::Green);
            }
        }
    }
    currentRadius = currentRadius - rateOfDecrease;
    currentIteration += 1;
    if (currentIteration >= iterationBeforeReseting) {
        currentRadius = EncirclementRadius;
        currentIteration = 0;
    }
    else if(currentRadius<=0){
        currentRadius = 0;
    }
    UE_LOG(LogTemp, Log, TEXT("After radius decrease: %.2f"), currentRadius);
    DrawDebugIndicators(world);
}

// M�thode pour dessiner un cercle de debug au-dessus des agents
void AiAgentGroupManager::DrawDebugIndicators(const UWorld* world)
{
    for (AActor* Agent : m_registeredAgents)
    {
        FVector AgentLocation = Agent->GetActorLocation();

        // Draw a yellow sphere above the AI agent
        DrawDebugSphere(
            world,
            AgentLocation + FVector(0.f, 0.f, 15.f), // Offset to place the sphere above the character
            20.0f,                                   // Sphere radius
            32,                                      // Sphere segments
            FColor::Yellow ,false,7                          // Sphere color
        );
    }
   
}
void AiAgentGroupManager::Destroy()
{
    delete m_Instance;
    m_Instance = nullptr;
}

void AiAgentGroupManager::RegisterAIAgent(ASoftDesignTrainingCharacter* aiAgent)
{
    UE_LOG(LogTemp, Log, TEXT("Register in manager"));
    if (m_registeredAgents.Contains(aiAgent)) {
        m_registeredAgents.Add(aiAgent);
    }
}

void AiAgentGroupManager::UnregisterAIAgent(ASoftDesignTrainingCharacter* aiAgent)
{

    UE_LOG(LogTemp, Log, TEXT("UnRegister in manager agent left: %d"), m_registeredAgents.Num());
    if (!m_registeredAgents.Contains(aiAgent)) {
        m_registeredAgents.Remove(aiAgent);
    }
    
}

TargetLKPInfo AiAgentGroupManager::GetLKPFromGroup(const FString& targetLabel, bool& targetfound)
{
    int agentCount = m_registeredAgents.Num();
    TargetLKPInfo outLKPInfo = TargetLKPInfo();
    targetfound = false;

    for (int i = 0; i < agentCount; i++)
    {
        ASoftDesignTrainingCharacter* aiAgent = m_registeredAgents[i];
        if (aiAgent)
        {
            const TargetLKPInfo& targetLKPInfo = aiAgent->GetCurrentTargetLKPInfo();
            if (targetLKPInfo.GetTargetLabel() == targetLabel)
            {
                if (targetLKPInfo.GetLastUpdatedTimeStamp() > outLKPInfo.GetLastUpdatedTimeStamp())
                {
                    targetfound = targetLKPInfo.GetLKPState() != TargetLKPInfo::ELKPState::LKPState_Invalid;
                    outLKPInfo = targetLKPInfo;
                }
            }
        }
    }

    return outLKPInfo;
}

