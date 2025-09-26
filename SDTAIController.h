#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SDTAIController.generated.h"

UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public AAIController
{
    GENERATED_BODY()
public:
    virtual void Tick(float deltaTime) override;

protected:
    // Détection joueur
    bool DetectPlayer(FVector& OutPlayerLocation) const;

    // Distance max de détection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
    float PlayerDetectDistance = 1000.f;
};
