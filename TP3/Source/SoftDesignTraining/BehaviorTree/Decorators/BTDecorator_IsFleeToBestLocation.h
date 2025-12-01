// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsFleeToBestLocation.generated.h"

/**
 *
 */
UCLASS()
class SOFTDESIGNTRAINING_API UBTDecorator_IsFleeToBestLocation : public UBTDecorator
{
	GENERATED_BODY()
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory) const override;
};
