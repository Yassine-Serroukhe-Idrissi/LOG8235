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

    // Son jou� quand le pickup est ramass�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
    USoundBase* PickupSound;

    // FX (particules) d�clench� quand le pickup est ramass�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feedback")
    UParticleSystem* PickupFX;

    // Fonction � appeler lors de la collecte
    UFUNCTION(BlueprintCallable, Category = "Feedback")
    void PlayFeedback();
};
