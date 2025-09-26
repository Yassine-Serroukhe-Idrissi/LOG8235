#include "SDTCollectible.h"
#include "SoftDesignTraining.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

ASDTCollectible::ASDTCollectible()
{
    PrimaryActorTick.bCanEverTick = true;

    // Création du composant feedback
    FeedbackComponent = CreateDefaultSubobject<UPickupFeedbackComponent>(TEXT("FeedbackComponent"));
}

void ASDTCollectible::BeginPlay()
{
    Super::BeginPlay();

    // Sauvegarde la position initiale
    initialPosition = GetActorLocation();
}

void ASDTCollectible::Collect()
{
    // Lance le timer de réapparition
    GetWorld()->GetTimerManager().SetTimer(
        m_CollectCooldownTimer,
        this,
        &ASDTCollectible::OnCooldownDone,
        m_CollectCooldownDuration,
        false
    );

    // Cache l’objet
    GetStaticMeshComponent()->SetVisibility(false);

    // Désactive les collisions
    GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // --- Joue le feedback (son + FX) ---
    if (FeedbackComponent)
    {
        FeedbackComponent->PlayFeedback();
    }
}

void ASDTCollectible::OnCooldownDone()
{
    GetWorld()->GetTimerManager().ClearTimer(m_CollectCooldownTimer);

    // Replace à la position initiale (utile si déplaçable)
    SetActorLocation(initialPosition);

    // Réactive l’objet
    GetStaticMeshComponent()->SetVisibility(true);
    GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

bool ASDTCollectible::IsOnCooldown()
{
    return m_CollectCooldownTimer.IsValid();
}

void ASDTCollectible::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    if (isMoveable)
    {
        FVector Offset(0.f, 0.f, FMath::Sin(GetWorld()->TimeSeconds * 2.f) * 10.f);
        SetActorLocation(initialPosition + Offset);
    }
}
