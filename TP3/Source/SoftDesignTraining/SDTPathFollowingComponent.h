// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/PathFollowingComponent.h"
#include "SDTPathFollowingComponent.generated.h"

/**
*
*/
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API USDTPathFollowingComponent : public UPathFollowingComponent
{
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
        float m_JumpProgressRatio = 0.f;

    virtual void FollowPathSegment(float DeltaTime) override;
    virtual void SetMoveSegment(int32 SegmentStartIndex) override;
};