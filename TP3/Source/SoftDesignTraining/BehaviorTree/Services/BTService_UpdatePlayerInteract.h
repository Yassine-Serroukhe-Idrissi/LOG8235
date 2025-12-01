#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdatePlayerInteract.generated.h"

UCLASS()
class SOFTDESIGNTRAINING_API UBTService_UpdatePlayerInteract : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_UpdatePlayerInteract();

protected:
    virtual void TickNode(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory, float DeltaSeconds) override;
};
