#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTTask_ShowNavigationPath.generated.h"

UCLASS()
class SOFTDESIGNTRAINING_API UBTTask_ShowNavigationPath : public UBTService
{
    GENERATED_BODY()

public:
    UBTTask_ShowNavigationPath();

protected:
    virtual void TickNode(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory, float DeltaSeconds) override;
};
