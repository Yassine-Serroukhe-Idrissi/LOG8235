// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTCollectible.h"
#include "SoftDesignTraining.h"

ASDTCollectible::ASDTCollectible()
{
    PrimaryActorTick.bCanEverTick = true;

    m_CollectionSound = nullptr;
    m_CollectionEffect = nullptr;
    m_SoundVolume = 1.0f;
}

void ASDTCollectible::BeginPlay()
{
    Super::BeginPlay();
    initialPosition = GetActorLocation();
}

void ASDTCollectible::Collect()
{
    TriggerCollectionFeedback();

    GetWorld()->GetTimerManager().SetTimer(m_CollectCooldownTimer, this, &ASDTCollectible::OnCooldownDone, m_CollectCooldownDuration, false);

    GetStaticMeshComponent()->SetVisibility(false);
}

void ASDTCollectible::OnCooldownDone()
{
    GetWorld()->GetTimerManager().ClearTimer(m_CollectCooldownTimer);

    GetStaticMeshComponent()->SetVisibility(true);
}

bool ASDTCollectible::IsOnCooldown()
{
    return m_CollectCooldownTimer.IsValid();
}

void ASDTCollectible::Tick(float deltaTime)
{
    Super::Tick(deltaTime);
}


void ASDTCollectible::TriggerCollectionFeedback()
{
    FVector CollectionLocation = GetActorLocation();

    // Play sound effect
    if (m_CollectionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            m_CollectionSound,
            CollectionLocation,
            m_SoundVolume
        );

        UE_LOG(LogTemp, Warning, TEXT("Playing collection sound"));
    }

    // Play particle effect
    if (m_CollectionEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            m_CollectionEffect,
            CollectionLocation,
            FRotator::ZeroRotator,
            FVector(1.0f),
            true
        );

        UE_LOG(LogTemp, Warning, TEXT("Playing collection effect"));
    }
}
