// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 *
 */
class SOFTDESIGNTRAINING_API TargetLKPInfo
{
public:
    TargetLKPInfo();
    TargetLKPInfo(const FString &targetLabel);
    ~TargetLKPInfo();

    enum class ELKPState : uint8
    {
        LKPState_ValidByLOS,
        LKPState_Valid,
        LKPState_Invalid
    };

    ELKPState GetLKPState() const { return m_currentLKPState; }
    void SetLKPState(ELKPState lkpState) { m_currentLKPState = lkpState; }

    const FVector &GetLKPPos() const { return m_pos; }
    void SetLKPPos(const FVector &lkpPos) { m_pos = lkpPos; }

    const FString &GetTargetLabel() const { return m_targetLabel; }
    void SetTargetLabel(const FString &targetLabel) { m_targetLabel = targetLabel; }

    float GetLastUpdatedTimeStamp() const { return m_lastUpdatedTimeStamp; }
    void SetLastUpdatedTimeStamp(float timeStamp) { m_lastUpdatedTimeStamp = timeStamp; }

private:
    ELKPState m_currentLKPState;
    FString m_targetLabel;
    FVector m_pos;
    float m_lastUpdatedTimeStamp;
};
