#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SDTAICharacter.generated.h"

UCLASS()
class SOFTDESIGNTRAINING_API ASDTAICharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASDTAICharacter();

    virtual void Tick(float DeltaTime) override;

    // --- Variables de déplacement ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Acceleration;

    // --- Détection des murs ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
    float WallDetectDistance;

    // --- Détection des DeathFloor ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
    float DeathFloorDetectDistance;

    // --- Détection des pickups ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
    float PickupDetectDistance;

private:
    FVector CurrentVelocity;

    void UpdateMovement(float DeltaTime);
    void OrientTowardsMovement(float DeltaTime);

    bool DetectWall(FVector& OutHitNormal) const;
    bool DetectDeathFloor(FVector& AvoidDirection) const;
    bool FindPickup(FVector& OutPickupLocation) const;
};


