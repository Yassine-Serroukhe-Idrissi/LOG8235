// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SDTUtils.h"
#include "SDTCollectible.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "Engine/Engine.h"

ASDTAIController::ASDTAIController()
{
    // Create components in constructor
    m_AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    m_ParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComponent"));

    if (m_ParticleComponent)
    {
        m_ParticleComponent->SetAutoActivate(false);
    }
}

void ASDTAIController::BeginPlay()
{
    Super::BeginPlay();

    // Attach components to pawn
    if (m_AudioComponent && GetPawn())
    {
        m_AudioComponent->AttachToComponent(GetPawn()->GetRootComponent(),
            FAttachmentTransformRules::KeepRelativeTransform);
    }

    if (m_ParticleComponent && GetPawn())
    {
        m_ParticleComponent->AttachToComponent(GetPawn()->GetRootComponent(),
            FAttachmentTransformRules::KeepRelativeTransform);
    }
}

void ASDTAIController::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return;

    FVector CurrentLocation = ControlledPawn->GetActorLocation();
    FVector NewDesiredDirection = m_DesiredDirection;
    float SpeedMultiplier = 1.0f;

    // Question 6 & 7: Player Detection and State Check
    bool bPlayerDetected = DetectPlayer();
    bool bPlayerPoweredUp = SDTUtils::IsPlayerPoweredUp(GetWorld());

    // Question 2 & 3: Wall and Death Floor Detection
    FVector WallAvoidanceDirection;
    FVector DeathFloorAvoidanceDirection;

    bool bWallDetected = DetectWalls(WallAvoidanceDirection);
    bool bDeathFloorDetected = DetectDeathFloors(DeathFloorAvoidanceDirection);

    // Question 4: Pickup Detection and Navigation
    if (!m_CurrentPickupTarget || !IsValid(m_CurrentPickupTarget))
    {
        m_CurrentPickupTarget = FindNearestPickup();
    }

    // Question 5: Check for pickup collection and trigger feedback
    if (m_CurrentPickupTarget)
    {
        float DistanceToPickup = FVector::Dist(CurrentLocation, m_CurrentPickupTarget->GetActorLocation());
        if (DistanceToPickup < m_PickupCollectionDistance)
        {
            TriggerCollectionFeedback(m_CurrentPickupTarget->GetActorLocation());
            m_CurrentPickupTarget = nullptr;
        }
    }

    // COMPLETE PRIORITY LOGIC WITH ALL QUESTIONS
    if (bWallDetected || bDeathFloorDetected)
    {
        // PRIORITY 1: Avoid obstacles (Questions 2 & 3)
        if (bWallDetected && bDeathFloorDetected)
        {
            NewDesiredDirection = (WallAvoidanceDirection + DeathFloorAvoidanceDirection).GetSafeNormal();
        }
        else if (bWallDetected)
        {
            NewDesiredDirection = WallAvoidanceDirection;
            SpeedMultiplier = 0.5f;
        }
        else
        {
            NewDesiredDirection = DeathFloorAvoidanceDirection;
        }
    }
    else if (bPlayerDetected && bPlayerPoweredUp && m_DetectedPlayer && IsPlayerTooClose())
    {
        // PRIORITY 2: Flee from powered up player (Question 7)
        FVector FleeDirection = CalculateFleeDirection(m_DetectedPlayer->GetActorLocation());
        NewDesiredDirection = FindBestFleeDirection(FleeDirection);
        SpeedMultiplier = m_FleeSpeed;

        UE_LOG(LogTemp, Warning, TEXT("AI fleeing from player - Player is powered up!"));
    }
    else if (bPlayerDetected && !bPlayerPoweredUp && m_DetectedPlayer && IsPathClearToPlayer(m_DetectedPlayer))
    {
        // PRIORITY 3: Chase unpowered player (Question 6)
        FVector PlayerDirection = CalculatePursuitDirection(m_DetectedPlayer->GetActorLocation());
        NewDesiredDirection = PlayerDirection;
        SpeedMultiplier = m_PlayerPursuitSpeed;

        UE_LOG(LogTemp, Warning, TEXT("AI chasing player - Player not powered up"));
    }
    else if (m_CurrentPickupTarget && IsPathClearToPickup(m_CurrentPickupTarget))
    {
        // PRIORITY 4: Navigate to pickups (Question 4)
        FVector PickupDirection = (m_CurrentPickupTarget->GetActorLocation() - CurrentLocation).GetSafeNormal();
        NewDesiredDirection = PickupDirection;
    }
    else
    {
        // PRIORITY 5: Default wandering behavior
        if (m_DesiredDirection.IsNearlyZero() || FMath::RandRange(0.0f, 1.0f) < 0.02f)
        {
            NewDesiredDirection = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal();
        }
        else
        {
            NewDesiredDirection = m_DesiredDirection;
        }
    }

    // Question 1: Physics-based movement with acceleration
    FVector DesiredVelocity = NewDesiredDirection * m_MaxVelocity * SpeedMultiplier;
    FVector VelocityChange = (DesiredVelocity - m_CurrentVelocity);

    // Apply acceleration limit using semi-implicit Euler integration
    if (VelocityChange.Size() > m_Acceleration * deltaTime)
    {
        VelocityChange = VelocityChange.GetSafeNormal() * m_Acceleration * deltaTime;
    }

    m_CurrentVelocity += VelocityChange;

    // Clamp to maximum velocity
    if (m_CurrentVelocity.Size() > m_MaxVelocity)
    {
        m_CurrentVelocity = m_CurrentVelocity.GetSafeNormal() * m_MaxVelocity;
    }

    // Add minimum velocity to prevent complete stops
    if (m_CurrentVelocity.Size() < 50.0f && NewDesiredDirection.Size() > 0.1f)
    {
        m_CurrentVelocity = NewDesiredDirection * 100.0f;
    }

    // Apply movement using physics system
    if (m_CurrentVelocity.Size() > 0.1f)
    {
        FVector MovementDirection = m_CurrentVelocity.GetSafeNormal();
        float MovementSpeed = m_CurrentVelocity.Size();

        // Use AddMovementInput from toolbox
        ControlledPawn->AddMovementInput(MovementDirection, MovementSpeed / m_MaxVelocity);

        // Rotate towards movement direction
        FRotator CurrentRotation = ControlledPawn->GetActorRotation();
        FRotator TargetRotation = MovementDirection.Rotation();
        TargetRotation.Pitch = CurrentRotation.Pitch;
        TargetRotation.Roll = CurrentRotation.Roll;

        FRotator DeltaRotation = (TargetRotation - CurrentRotation).GetNormalized();
        float MaxRotationThisFrame = m_RotationSpeed * deltaTime;

        if (FMath::Abs(DeltaRotation.Yaw) > MaxRotationThisFrame)
        {
            DeltaRotation.Yaw = FMath::Sign(DeltaRotation.Yaw) * MaxRotationThisFrame;
        }

        // Use AddActorWorldRotation from toolbox
        ControlledPawn->AddActorWorldRotation(FRotator(0, DeltaRotation.Yaw, 0));
    }

    m_DesiredDirection = NewDesiredDirection;
}

// Questions 2 & 3: Wall and Death Floor Detection
bool ASDTAIController::DetectWalls(FVector& AvoidanceDirection)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return false;

    FVector CurrentLocation = ControlledPawn->GetActorLocation();
    FVector ForwardVector = ControlledPawn->GetActorForwardVector();
    FVector RightVector = ControlledPawn->GetActorRightVector();

    TArray<FVector> RayDirections;
    RayDirections.Add(ForwardVector);
    RayDirections.Add((ForwardVector + RightVector * 0.5f).GetSafeNormal());
    RayDirections.Add((ForwardVector - RightVector * 0.5f).GetSafeNormal());
    RayDirections.Add(RightVector);
    RayDirections.Add(-RightVector);

    FVector TotalAvoidance = FVector::ZeroVector;
    bool bObstacleDetected = false;

    for (const FVector& Direction : RayDirections)
    {
        FVector EndPoint = CurrentLocation + Direction * m_WallDetectionDistance;

        if (SDTUtils::Raycast(GetWorld(), CurrentLocation, EndPoint))
        {
            bObstacleDetected = true;
            FVector AvoidDir = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
            if (FVector::DotProduct(AvoidDir, RightVector) < 0)
            {
                AvoidDir = -AvoidDir;
            }
            TotalAvoidance += AvoidDir;
        }
    }

    if (bObstacleDetected)
    {
        AvoidanceDirection = TotalAvoidance.GetSafeNormal();
        return true;
    }

    return false;
}

bool ASDTAIController::DetectDeathFloors(FVector& AvoidanceDirection)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return false;

    FVector CurrentLocation = ControlledPawn->GetActorLocation();
    FVector ForwardVector = ControlledPawn->GetActorForwardVector();

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(ControlledPawn);

    FVector StartPoint = CurrentLocation + ForwardVector * 50.0f;
    FVector EndPoint = StartPoint + FVector::DownVector * 200.0f;

    bool bHit = GetWorld()->LineTraceSingleByObjectType(
        HitResult,
        StartPoint,
        EndPoint,
        FCollisionObjectQueryParams(COLLISION_DEATH_OBJECT),
        QueryParams
    );

    if (bHit)
    {
        FVector HitNormal = HitResult.Normal;
        AvoidanceDirection = CalculateAvoidanceDirection(HitNormal);
        return true;
    }

    return false;
}

// Question 4: Pickup Detection
AActor* ASDTAIController::FindNearestPickup()
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return nullptr;

    FVector CurrentLocation = ControlledPawn->GetActorLocation();
    AActor* NearestPickup = nullptr;
    float NearestDistance = m_PickupDetectionDistance;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTCollectible::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (ASDTCollectible* Collectible = Cast<ASDTCollectible>(Actor))
        {
            if (!Collectible->IsOnCooldown())
            {
                float Distance = FVector::Dist(CurrentLocation, Actor->GetActorLocation());
                if (Distance < NearestDistance)
                {
                    NearestDistance = Distance;
                    NearestPickup = Actor;
                }
            }
        }
    }

    return NearestPickup;
}

bool ASDTAIController::IsPathClearToPickup(AActor* Pickup)
{
    if (!Pickup)
        return false;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return false;

    FVector StartLocation = ControlledPawn->GetActorLocation();
    FVector EndLocation = Pickup->GetActorLocation();

    return !SDTUtils::Raycast(GetWorld(), StartLocation, EndLocation);
}

FVector ASDTAIController::CalculateAvoidanceDirection(FVector HitNormal)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return FVector::ForwardVector;

    FVector RightVector = ControlledPawn->GetActorRightVector();
    FVector AvoidanceDir = FVector::CrossProduct(HitNormal, FVector::UpVector).GetSafeNormal();

    if (FVector::DotProduct(AvoidanceDir, RightVector) < 0)
    {
        AvoidanceDir = -AvoidanceDir;
    }

    return AvoidanceDir;
}

// Question 5: Collection Feedback
void ASDTAIController::TriggerCollectionFeedback(const FVector& CollectionLocation)
{
    if (m_EnableSoundFeedback)
    {
        PlayCollectionSound(CollectionLocation);
    }

    if (m_EnableVisualFeedback)
    {
        PlayCollectionEffect(CollectionLocation);
    }

    UE_LOG(LogTemp, Warning, TEXT("AI collected pickup at location: %s"), *CollectionLocation.ToString());
}

void ASDTAIController::PlayCollectionSound(const FVector& Location)
{
    if (!m_CollectionSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("Collection sound not set in AI Controller"));
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        m_CollectionSound,
        Location,
        m_SoundVolume
    );

    if (m_AudioComponent && m_AudioComponent->IsValidLowLevel())
    {
        m_AudioComponent->SetSound(m_CollectionSound);
        m_AudioComponent->SetVolumeMultiplier(m_SoundVolume);
        m_AudioComponent->Play();
    }
}

void ASDTAIController::PlayCollectionEffect(const FVector& Location)
{
    if (!m_CollectionEffect)
    {
        UE_LOG(LogTemp, Warning, TEXT("Collection particle effect not set in AI Controller"));
        return;
    }

    UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        m_CollectionEffect,
        Location,
        FRotator::ZeroRotator,
        FVector(1.0f),
        true,
        EPSCPoolMethod::None
    );

    if (m_ParticleComponent && m_ParticleComponent->IsValidLowLevel())
    {
        m_ParticleComponent->SetTemplate(m_CollectionEffect);
        m_ParticleComponent->SetWorldLocation(Location);
        m_ParticleComponent->Activate(true);
    }
}

// Question 6: Player Pursuit
bool ASDTAIController::DetectPlayer()
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return false;

    FVector CurrentLocation = ControlledPawn->GetActorLocation();

    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerCharacter)
    {
        m_DetectedPlayer = nullptr;
        m_IsPlayerInRange = false;
        return false;
    }

    float DistanceToPlayer = FVector::Dist(CurrentLocation, PlayerCharacter->GetActorLocation());

    if (DistanceToPlayer <= m_PlayerDetectionDistance)
    {
        m_DetectedPlayer = PlayerCharacter;
        m_IsPlayerInRange = true;

        // Debug visuals
        DrawDebugSphere(GetWorld(), CurrentLocation, m_PlayerDetectionDistance, 12, FColor::Red, false, 0.1f);
        DrawDebugLine(GetWorld(), CurrentLocation, PlayerCharacter->GetActorLocation(), FColor::Yellow, false, 0.1f, 0, 2.0f);

        return true;
    }
    else
    {
        m_DetectedPlayer = nullptr;
        m_IsPlayerInRange = false;
        return false;
    }
}

bool ASDTAIController::IsPathClearToPlayer(AActor* Player)
{
    if (!Player)
        return false;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return false;

    FVector StartLocation = ControlledPawn->GetActorLocation();
    FVector EndLocation = Player->GetActorLocation();

    return !SDTUtils::Raycast(GetWorld(), StartLocation, EndLocation);
}

FVector ASDTAIController::CalculatePursuitDirection(const FVector& PlayerLocation)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return FVector::ForwardVector;

    FVector CurrentLocation = ControlledPawn->GetActorLocation();
    FVector DirectionToPlayer = (PlayerLocation - CurrentLocation).GetSafeNormal();

    // Basic prediction of player movement
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(m_DetectedPlayer))
    {
        FVector PlayerVelocity = PlayerCharacter->GetVelocity();
        if (PlayerVelocity.Size() > 0.1f)
        {
            FVector PredictedPlayerLocation = PlayerLocation + PlayerVelocity * 0.5f;
            DirectionToPlayer = (PredictedPlayerLocation - CurrentLocation).GetSafeNormal();
        }
    }

    return DirectionToPlayer;
}

// Question 7: Flee Behavior
FVector ASDTAIController::CalculateFleeDirection(const FVector& PlayerLocation)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return FVector::ForwardVector;

    FVector CurrentLocation = ControlledPawn->GetActorLocation();

    // Calculate direction AWAY from player
    FVector FleeDirection = (CurrentLocation - PlayerLocation).GetSafeNormal();

    // Add some variability to avoid oscillations
    FVector RandomOffset = FVector(
        FMath::RandRange(-0.3f, 0.3f),
        FMath::RandRange(-0.3f, 0.3f),
        0.0f
    );

    FleeDirection = (FleeDirection + RandomOffset).GetSafeNormal();

    return FleeDirection;
}

bool ASDTAIController::IsPlayerTooClose()
{
    if (!m_DetectedPlayer)
        return false;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return false;

    FVector CurrentLocation = ControlledPawn->GetActorLocation();
    FVector PlayerLocation = m_DetectedPlayer->GetActorLocation();

    float DistanceToPlayer = FVector::Dist(CurrentLocation, PlayerLocation);

    // Player is "too close" if within detection range
    return (DistanceToPlayer < m_PlayerDetectionDistance);
}

FVector ASDTAIController::FindBestFleeDirection(const FVector& BaseFleeDirection)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return BaseFleeDirection;

    FVector CurrentLocation = ControlledPawn->GetActorLocation();

    // Test multiple flee directions to find the best one
    TArray<FVector> FleeOptions;
    FleeOptions.Add(BaseFleeDirection); // Base direction (away from player)
    FleeOptions.Add((BaseFleeDirection + ControlledPawn->GetActorRightVector() * 0.7f).GetSafeNormal()); // Right
    FleeOptions.Add((BaseFleeDirection - ControlledPawn->GetActorRightVector() * 0.7f).GetSafeNormal()); // Left
    FleeOptions.Add((BaseFleeDirection + ControlledPawn->GetActorRightVector()).GetSafeNormal()); // More right
    FleeOptions.Add((BaseFleeDirection - ControlledPawn->GetActorRightVector()).GetSafeNormal()); // More left

    // Test each option and choose first one without obstacles
    for (const FVector& FleeOption : FleeOptions)
    {
        FVector TestEndPoint = CurrentLocation + FleeOption * m_WallDetectionDistance;

        // Check if there's no obstacle in this direction
        if (!SDTUtils::Raycast(GetWorld(), CurrentLocation, TestEndPoint))
        {
            // Debug visual for chosen flee direction
            DrawDebugLine(GetWorld(), CurrentLocation, TestEndPoint, FColor::Blue, false, 0.1f, 0, 3.0f);
            return FleeOption;
        }
    }

    // If no direction is clear, use base direction
    UE_LOG(LogTemp, Warning, TEXT("No clear flee direction found, using base direction"));
    return BaseFleeDirection;
}
