#include "SDTAICharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "SDTUtils.h"

ASDTAICharacter::ASDTAICharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    MaxSpeed = 600.f;
    Acceleration = 800.f;
    WallDetectDistance = 200.f;
    DeathFloorDetectDistance = 150.f;

    CurrentVelocity = FVector::ZeroVector;
}

void ASDTAICharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateMovement(DeltaTime);
    OrientTowardsMovement(DeltaTime);
}

void ASDTAICharacter::UpdateMovement(float DeltaTime)
{
    FVector DesiredDirection = GetActorForwardVector();

    // --- D�tection mur ---
    FVector HitNormal;
    if (DetectWall(HitNormal))
    {
        DesiredDirection = FVector::VectorPlaneProject(GetActorForwardVector(), HitNormal).GetSafeNormal();
    }

    // --- D�tection DeathFloor ---
    FVector AvoidDir;
    if (DetectDeathFloor(AvoidDir))
    {
        DesiredDirection = AvoidDir;
    }

    // Vitesse cible
    FVector DesiredVelocity = DesiredDirection * MaxSpeed;

    // Acc�l�ration progressive
    FVector Delta = DesiredVelocity - CurrentVelocity;
    FVector AccelStep = Delta.GetClampedToMaxSize(Acceleration * DeltaTime);
    CurrentVelocity += AccelStep;

    // D�placement
    AddMovementInput(CurrentVelocity.GetSafeNormal(), CurrentVelocity.Size() / MaxSpeed);
}

void ASDTAICharacter::OrientTowardsMovement(float DeltaTime)
{
    if (CurrentVelocity.IsNearlyZero())
        return;

    FRotator TargetRotation = CurrentVelocity.Rotation();
    FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 5.f);

    SetActorRotation(NewRotation);
}

bool ASDTAICharacter::DetectWall(FVector& OutHitNormal) const
{
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * WallDetectDistance;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit, Start, End, ECC_WorldStatic, Params
    );

#if WITH_EDITOR
    DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Red : FColor::Green, false, 0.1f, 0, 2.0f);
#endif

    if (bHit)
    {
        OutHitNormal = Hit.ImpactNormal;
        return true;
    }

    return false;
}

bool ASDTAICharacter::DetectDeathFloor(FVector& AvoidDirection) const
{
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * DeathFloorDetectDistance;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit, Start, End, COLLISION_DEATH_OBJECT, Params
    );

#if WITH_EDITOR
    DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Orange : FColor::Blue, false, 0.1f, 0, 2.0f);
#endif

    if (bHit)
    {
        // �vite la dalle en tournant sur le c�t�
        AvoidDirection = FVector::CrossProduct(FVector::UpVector, GetActorForwardVector()).GetSafeNormal();
        return true;
    }

    return false;
}
