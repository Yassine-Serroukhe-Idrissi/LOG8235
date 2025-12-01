// Fill out your copyright notice in the Description page of Project Settings.

// SDTBaseAIController.cpp
#include "SDTBaseAIController.h"
#include "SoftDesignTraining.h"
#include "AiAgentGroupManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Controller.h"  
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"

float ASDTBaseAIController::TotalTimeBudgetUsed = 0.0f;
const float ASDTBaseAIController::MaxTimeBudgetPerFrame = 0.002f;
int32 ASDTBaseAIController::AgentsUpdatedThisFrame = 0;

ASDTBaseAIController::ASDTBaseAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    m_ReachedTarget = true;
    bWasVisibleLastFrame = true;
    bShouldTickThisFrame = true;
}

void ASDTBaseAIController::Tick(float deltaTime)
{
    if (!GetWorld())
        return;

    // Réinitialiser le compteur au début de chaque frame
    static uint64 LastFrameNumber = 0;
    uint64 CurrentFrameNumber = GFrameCounter;

    if (CurrentFrameNumber != LastFrameNumber)
    {
        TotalTimeBudgetUsed = 0.0f;
        AgentsUpdatedThisFrame = 0;
        LastFrameNumber = CurrentFrameNumber;
    }

    // Vérifier le budget disponible
    if (TotalTimeBudgetUsed >= MaxTimeBudgetPerFrame)
    {
        // Budget épuisé, on skip cet agent cette frame
        return;
    }

    // Mesurer le temps de cette mise à jour
    double StartTime = FPlatformTime::Seconds();

    // Tick normal
    Super::Tick(deltaTime);

    // Mise à jour du tick rate dynamique
    UpdateTickRate();

    // Calculer le temps utilisé
    double EndTime = FPlatformTime::Seconds();
    float TimeUsed = static_cast<float>(EndTime - StartTime);
    TotalTimeBudgetUsed += TimeUsed;
    AgentsUpdatedThisFrame++;
}

bool ASDTBaseAIController::IsVisibleInCameraView()
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->PlayerCameraManager)
        return false;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return false;

    FVector CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
    FRotator CameraRotation = PlayerController->PlayerCameraManager->GetCameraRotation();

    FVector PawnLocation = ControlledPawn->GetActorLocation();
    FVector DirectionToAI = (PawnLocation - CameraLocation).GetSafeNormal();
    FVector CameraForward = CameraRotation.Vector();

    // Vérifier si dans le FOV
    float DotProduct = FVector::DotProduct(CameraForward, DirectionToAI);
    float FOVAngle = PlayerController->PlayerCameraManager->GetFOVAngle();
    float MinDot = FMath::Cos(FMath::DegreesToRadians(FOVAngle / 2.0f));

    // Aussi vérifier la distance pour ne pas ralentir les agents trop proches
    float Distance = FVector::Dist(CameraLocation, PawnLocation);

    return (DotProduct >= MinDot) || (Distance < 500.0f);
}

void ASDTBaseAIController::UpdateTickRate()
{
    if (!GetPawn() || !GetWorld())
        return;

    bool bIsVisible = IsVisibleInCameraView();

    // Ne changer que si le statut change
    if (bIsVisible != bWasVisibleLastFrame)
    {
        ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn());
        if (!ControlledCharacter)
            return;

        if (bIsVisible)
        {
            // Agent visible : tick normal
            SetActorTickInterval(0.0f);

            // Animations à pleine vitesse
            if (USkeletalMeshComponent* MeshComp = ControlledCharacter->GetMesh())
            {
                MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
                MeshComp->SetComponentTickInterval(0.0f);
            }
        }
        else
        {
            // Agent non visible : tick réduit
            SetActorTickInterval(0.2f);

            // Réduire les animations
            if (USkeletalMeshComponent* MeshComp = ControlledCharacter->GetMesh())
            {
                MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
                MeshComp->SetComponentTickInterval(0.2f);
            }
        }

        bWasVisibleLastFrame = bIsVisible;
    }
}