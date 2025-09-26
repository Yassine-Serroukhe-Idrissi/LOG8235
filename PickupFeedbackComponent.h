#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PickupFeedbackComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOFTDESIGNTRAINING_API UPickupFeedbackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPickupFeedbackComponent();

    // Son joué quand le pickup est ramassé
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
    USoundBase* PickupSound;

    // FX (particules) déclenché quand le pickup est ramassé
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
    UParticleSystem* PickupFX;

    // Fonction à appeler lors de la collecte
    UFUNCTION(BlueprintCallable, Category = "Feedback")
    void PlayFeedback();
};
