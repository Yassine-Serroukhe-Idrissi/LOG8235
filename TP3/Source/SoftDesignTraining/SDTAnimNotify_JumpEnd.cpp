// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAnimNotify_JumpEnd.h"
#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"

void USDTAnimNotify_JumpEnd::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
    if (AActor* owner = MeshComp->GetOwner())
    {
        if (ASoftDesignTrainingCharacter* character = Cast<ASoftDesignTrainingCharacter>(owner))
        {
            if (ASDTAIController* controller = Cast<ASDTAIController>(character->GetController()))
            {
                controller->InAir = false;
                controller->Landing = true;
            }
        }
    }
}
