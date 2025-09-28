#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SDTUtils.h"
#include "SDTCollectible.h"
#include "SoftDesignTrainingMainCharacter.h"

void ASDTAIController::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    ChaseOrFleePlayer(deltaTime);
    ChaseCollectible(deltaTime);
    AvoidObstacles(deltaTime);
    MovePawn(deltaTime);
}

void ASDTAIController::MovePawn(float deltaTime)
{
    APawn* pawn = GetPawn();
    if (!pawn)
        return;

    CurrentVelocity += pawn->GetActorForwardVector() * Acceleration * deltaTime;
    CurrentVelocity = CurrentVelocity.GetClampedToMaxSize(MaxSpeed);

    if (!CurrentVelocity.IsNearlyZero())
    {
        pawn->AddMovementInput(CurrentVelocity.GetSafeNormal(), CurrentVelocity.Size());

        // Calcul de la rotation cible basee sur la direction de la vitesse
        FRotator TargetRotation = CurrentVelocity.Rotation();
        TargetRotation.Pitch = 0.f;
        TargetRotation.Roll = 0.f;

        // Interpolation entre rotation actuelle et cible
        FRotator CurrentRotation = pawn->GetActorRotation();
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, deltaTime, RotationSpeed);
        pawn->SetActorRotation(NewRotation);
    }
}

void ASDTAIController::AvoidObstacles(float deltaTime)
{
    APawn* pawn = GetPawn();
    if (!pawn) return;

    FVector Start = pawn->GetActorLocation();
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(pawn);

    FVector Forward = pawn->GetActorForwardVector();

    TArray<float> Angles = { 0.f, 30.f, -30.f, 60.f, -60.f };

    FVector Accumulated = FVector::ZeroVector;
    float TotalWeight = 0.f;

    for (float Angle : Angles)
    {
        FVector Direction = Forward.RotateAngleAxis(Angle, FVector::UpVector).GetSafeNormal();

        // Raycast pour detecter un mur devant
        FVector EndWall = Start + Direction * DetectionDistance;
        FHitResult WallHit;
        bool bWallHit = GetWorld()->LineTraceSingleByChannel(WallHit, Start, EndWall, ECC_WorldStatic, Params);
        float WallDistance = bWallHit ? WallHit.Distance : DetectionDistance;

        // Raycast vertical pour detecter si le sol est death floor
        FVector ProbeStart = Start + Direction * DetectionDistance;
        FVector ProbeEnd = ProbeStart + FVector::DownVector * FloorDetectionDistance;
        FHitResult FloorHit;
        bool bDeadly = GetWorld()->LineTraceSingleByObjectType(
            FloorHit, ProbeStart, ProbeEnd, COLLISION_DEATH_OBJECT, Params);

        if (bDeadly)
        {
            // Eviter fortement les directions qui menent au death floor
            Accumulated -= Direction * 2.f;
            DrawDebugLine(GetWorld(), ProbeStart, ProbeEnd, FColor::Red, false, 0.f, 0, 2.f);
            continue;
        }

        // Poids ajuste selon distance disponible avant obstacle
        float Clearance = 100.f;
        float EffectiveDistance = FMath::Max(0.f, WallDistance - Clearance);
        float Weight = EffectiveDistance / DetectionDistance;

        Weight = FMath::Pow(Weight, 2.f);

        Accumulated += Direction * Weight;
        TotalWeight += Weight;

        DrawDebugLine(GetWorld(), Start, EndWall, bWallHit ? FColor::Red : FColor::Green, false, 0.f, 0, 2.f);
        DrawDebugLine(GetWorld(), ProbeStart, ProbeEnd, FColor::Blue, false, 0.f, 0, 2.f);
    }

    FVector BestDirection;
    if (TotalWeight > KINDA_SMALL_NUMBER)
    {
        BestDirection = (Accumulated / TotalWeight).GetSafeNormal();
    }
    else
    {
        BestDirection = Forward;
    }

    // Interpolation entre direction actuelle et meilleure direction
    FVector CurrentDirection = CurrentVelocity.GetSafeNormal();
    FVector NewDirection = FMath::VInterpTo(CurrentDirection, BestDirection, deltaTime, RotationSpeed).GetSafeNormal();
    float Speed = CurrentVelocity.Size();

    CurrentVelocity = NewDirection * Speed;
    LastAvoidanceDirection = BestDirection;
}

void ASDTAIController::ChaseCollectible(float deltaTime)
{
    APawn* pawn = GetPawn();
    if (!pawn) return;

    FVector Start = pawn->GetActorLocation();
    FVector Forward = pawn->GetActorForwardVector();

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(pawn);

    float BestDistance = DetectionDistance;
    FVector BestDirection = FVector::ZeroVector;

    TArray<float> Angles = { 0.f, 15.f, -15.f, 30.f, -30.f, 45.f, -45.f };
    for (float Angle : Angles)
    {
        FVector Direction = Forward.RotateAngleAxis(Angle, FVector::UpVector).GetSafeNormal();
        FVector End = Start + Direction * DetectionDistance;

        // Raycast pour pickups
        FHitResult CollectibleHit;
        bool bCollectible = GetWorld()->LineTraceSingleByObjectType(
            CollectibleHit, Start, End,
            FCollisionObjectQueryParams(COLLISION_COLLECTIBLE), Params);

        if (bCollectible)
        {
            // Ignore si le pickup est en cooldown
            if (ASDTCollectible* Collectible = Cast<ASDTCollectible>(CollectibleHit.GetActor()))
            {
                if (Collectible->IsOnCooldown())
                    continue;
            }

            // Verifie qu'aucun mur bloque la vue
            FHitResult WallHit;
            bool bWall = GetWorld()->LineTraceSingleByChannel(
                WallHit, Start, CollectibleHit.ImpactPoint, ECC_WorldStatic, Params);

            if (!bWall && CollectibleHit.Distance < BestDistance)
            {
                BestDistance = CollectibleHit.Distance;
                BestDirection = Direction;
            }

            DrawDebugLine(GetWorld(), Start, CollectibleHit.ImpactPoint,
                FColor::Yellow, false, 0.f, 0, 2.f);
        }
    }

    if (!BestDirection.IsNearlyZero())
    {
        CurrentVelocity = FMath::VInterpTo(
            CurrentVelocity, BestDirection * MaxSpeed, deltaTime, RotationSpeed);
    }
}

void ASDTAIController::ChaseOrFleePlayer(float deltaTime)
{
    APawn* pawn = GetPawn();
    if (!pawn) return;

    FVector Start = pawn->GetActorLocation();
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(pawn);

    FHitResult PlayerHit;
    bool bPlayerDetected = GetWorld()->SweepSingleByObjectType(
        PlayerHit,
        Start,
        Start,
        FQuat::Identity,
        FCollisionObjectQueryParams(COLLISION_PLAYER),
        FCollisionShape::MakeSphere(PlayerDetectionRadius),
        Params
    );

    DrawDebugSphere(
        GetWorld(),
        Start,
        PlayerDetectionRadius,
        16,
        bPlayerDetected ? FColor::Red : FColor::Green,
        false,
        0.f,
        0,
        2.f
    );

    if (bPlayerDetected)
    {
        if (ASoftDesignTrainingMainCharacter* Player =
            Cast<ASoftDesignTrainingMainCharacter>(PlayerHit.GetActor()))
        {
            FVector ToPlayer = (Player->GetActorLocation() - Start).GetSafeNormal();
            FVector DesiredDirection;

            if (Player->IsPoweredUp())
            {
                DesiredDirection = -ToPlayer;
            }
            else
            {
                DesiredDirection = ToPlayer;
            }

            // Verifie si la direction mene a un death floor
            FVector ProbeStart = Start + DesiredDirection * DetectionDistance;
            FVector ProbeEnd = ProbeStart + FVector::DownVector * FloorDetectionDistance;
            FHitResult FloorHit;
            bool bDeadly = GetWorld()->LineTraceSingleByObjectType(
                FloorHit,
                ProbeStart,
                ProbeEnd,
                COLLISION_DEATH_OBJECT,
                Params
            );

            // Si death floor, on reutilise la derniere bonne direction
            FVector SafeDirection = LastAvoidanceDirection.IsNearlyZero()
                ? pawn->GetActorForwardVector()
                : LastAvoidanceDirection;

            FVector FinalDirection;
            if (bDeadly)
            {
                FinalDirection = SafeDirection;
            }
            else
            {
                // Combine direction vers joueur et bonne direction
                float DistanceToPlayer = FVector::Dist(Player->GetActorLocation(), Start);
                float AvoidScale = FMath::Clamp(DistanceToPlayer / 500.f, 0.f, 1.f);
                float PlayerWeight = 1.5f;
                float AvoidWeight = 2.0f * AvoidScale;
                FinalDirection = (DesiredDirection * PlayerWeight + SafeDirection * AvoidWeight).GetSafeNormal();
            }

            // Interpolation vers la direction finale
            CurrentVelocity = FMath::VInterpTo(
                CurrentVelocity,
                FinalDirection * MaxSpeed,
                deltaTime,
                RotationSpeed
            );
        }
    }
}