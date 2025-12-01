// Fill out your copyright notice in the Description page of Project Settings.

#include "TargetLKPInfo.h"
#include "SoftDesignTraining.h"

TargetLKPInfo::TargetLKPInfo()
    : m_currentLKPState(ELKPState::LKPState_Invalid), m_pos(FVector::ZeroVector), m_lastUpdatedTimeStamp(-1.f)
{
}

TargetLKPInfo::TargetLKPInfo(const FString &targetLabel)
{
    m_targetLabel = targetLabel;
}

TargetLKPInfo::~TargetLKPInfo()
{
}
