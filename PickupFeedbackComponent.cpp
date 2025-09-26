#include "PickupFeedbackComponent.h"
#include "Kismet/GameplayStatics.h"

UPickupFeedbackComponent::UPickupFeedbackComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPickupFeedbackComponent::PlayFeedback()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, PickupSound, Owner->GetActorLocation());
    }

    if (PickupFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PickupFX, Owner->GetActorTransform());
    }
}
