#include "SDTAIController.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "SDTUtils.h"

void ASDTAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector PlayerLocation;
    if (DetectPlayer(PlayerLocation))
    {
        APawn* ControlledPawn = GetPawn();
        if (!ControlledPawn) return;

        // Vérifie si le joueur est en mode PowerUp
        bool bPlayerPoweredUp = SDTUtils::IsPlayerPoweredUp(GetWorld());

        FVector MyLocation = ControlledPawn->GetActorLocation();
        FVector Direction;

        if (bPlayerPoweredUp)
        {
            // --- Fuite ---
            Direction = (MyLocation - PlayerLocation).GetSafeNormal();

#if WITH_EDITOR
            DrawDebugLine(GetWorld(), MyLocation, PlayerLocation, FColor::Blue, false, 0.1f, 0, 2.0f);
#endif
        }
        else
        {
            // --- Poursuite ---
            Direction = (PlayerLocation - MyLocation).GetSafeNormal();

#if WITH_EDITOR
            DrawDebugLine(GetWorld(), MyLocation, PlayerLocation, FColor::Red, false, 0.1f, 0, 2.0f);
#endif
        }

        ControlledPawn->AddMovementInput(Direction, 1.0f);
    }
}

bool ASDTAIController::DetectPlayer(FVector& OutPlayerLocation) const
{
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerCharacter)
        return false;

    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector PlayerPos = PlayerCharacter->GetActorLocation();

    float Dist = FVector::Dist(MyLocation, PlayerPos);
    if (Dist > PlayerDetectDistance)
        return false;

    // Vérifie la ligne de vue
    if (!SDTUtils::Raycast(GetWorld(), MyLocation, PlayerPos))
    {
        OutPlayerLocation = PlayerPos;
        return true;
    }

    return false;
}
