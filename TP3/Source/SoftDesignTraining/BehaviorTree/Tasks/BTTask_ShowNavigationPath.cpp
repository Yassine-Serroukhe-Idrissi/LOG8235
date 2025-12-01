#include "BTTask_ShowNavigationPath.h"
#include "SoftDesignTraining/SDTAIController.h"

UBTTask_ShowNavigationPath::UBTTask_ShowNavigationPath()
{
    bNotifyTick = true; // Enable ticking for this service
}

void UBTTask_ShowNavigationPath::TickNode(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory, float DeltaSeconds)
{
    ASDTAIController *AIController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        return; // Early exit if the controller is not valid
    }

    AIController->ShowNavigationPath();
}
