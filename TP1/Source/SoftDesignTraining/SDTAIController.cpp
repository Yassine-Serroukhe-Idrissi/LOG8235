// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SDTUtils.h"



void ASDTAIController::Tick(float deltaTime)
{
	Super::Tick(deltaTime);
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
		return;

	bool bWallDetected = DetectWallAhead(deltaTime);
	if (bWallDetected) {
		AdjustSpeedForWallAvoidance(deltaTime);
		PerformWallAvoidanceRotation(deltaTime);
	}
	else {
		bIsAvoidingWall = false;
		CurrentSpeedMultiplier = 1.0f;
	}

	CalculateVelocity(deltaTime);
	ApplyMovement(deltaTime);

	if (!bIsAvoidingWall) {
		UpdateOrientation(deltaTime);
	}
}

void ASDTAIController::CalculateVelocity(float deltaTime) {
	FVector DesiredDirection = GetPawn()->GetActorForwardVector().GetSafeNormal();
	float AdjustMaxSpeed = MaxSpeed * CurrentSpeedMultiplier;
	FVector TargetVelocity = DesiredDirection * AdjustMaxSpeed;
	FVector VelocityDiffrence = TargetVelocity - CurrentVelocity;
	FVector AccelerationVector = VelocityDiffrence.GetSafeNormal() * Acceleration;
	FVector VelocityChange = AccelerationVector * deltaTime;
	if (VelocityChange.Size() > VelocityDiffrence.Size())
		VelocityChange = VelocityDiffrence;
	CurrentVelocity += VelocityChange;
	if (CurrentVelocity.Size() > AdjustMaxSpeed)
		CurrentVelocity = CurrentVelocity.GetSafeNormal() * AdjustMaxSpeed;

}

void ASDTAIController::ApplyMovement(float deltaTime) {
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || CurrentVelocity.Size()<0.1f)
		return;
	FVector MovementDirection = CurrentVelocity.GetSafeNormal();
	float MovementIntensity = FMath::Clamp(CurrentVelocity.Size() / MaxSpeed, 0.0f, 1.0f);
	ControlledPawn->AddMovementInput(MovementDirection, MovementIntensity);

}

void ASDTAIController::UpdateOrientation(float deltaTime) {
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || CurrentVelocity.Size() < 0.1f)
		return;
	FVector MovementDirection = CurrentVelocity.GetSafeNormal();
	FRotator TargetRotation = MovementDirection.Rotation();

	FRotator CurrentRotation = ControlledPawn->GetActorRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, deltaTime, RotationSpeed / 180.0f * PI);
	ControlledPawn->SetActorRotation(NewRotation);
}

bool ASDTAIController::DetectWallAhead(float deltaTime) {
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
		return false;
	FVector PawnLocation = ControlledPawn->GetActorLocation();
	FVector ForwardVector = ControlledPawn->GetActorForwardVector();

	FVector StartPoint = PawnLocation;
	FVector EndPoint = PawnLocation + (ForwardVector * WallDetectionDistance);

	bool bHitDetected = SDTUtils::Raycast(GetWorld(), StartPoint, EndPoint);

	if (bHitDetected) {
		FHitResult  HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(ControlledPawn);

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
		FCollisionObjectQueryParams ObjectQueryParams(ObjectTypes);
		bool bDetailedHit = GetWorld()->LineTraceSingleByObjectType(
			HitResult,
			StartPoint,
			EndPoint,
			ObjectQueryParams,
			CollisionParams
		);
		if (bDetailedHit) {
			WallNormal = HitResult.ImpactNormal;
			bIsAvoidingWall = true;
			return true;
		}

	}
	return false;
}

void ASDTAIController::AdjustSpeedForWallAvoidance(float deltaTime) {
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || CurrentVelocity.Size() < 0.1f)
		return;
	FVector PawnLocation = ControlledPawn->GetActorLocation();
	FVector FarwardVector = ControlledPawn->GetActorForwardVector();
	FVector StartPoint = PawnLocation;
	FVector EndPoint = PawnLocation + (FarwardVector * WallDetectionDistance);

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(ControlledPawn);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	FCollisionObjectQueryParams ObjectQueryParams(ObjectTypes);

	bool bHit = GetWorld()->LineTraceSingleByObjectType(
		HitResult,
		StartPoint,
		EndPoint,
		ObjectQueryParams,
		CollisionParams
	);
	if (bHit) {
		float DistanceToWall = HitResult.Distance;
		float SpeedRatio = FMath::Clamp(DistanceToWall / WallDetectionDistance, MinSpeedWhenAvoiding, 1.0f);
		CurrentSpeedMultiplier = SpeedRatio;
	}
}

void ASDTAIController::PerformWallAvoidanceRotation(float deltaTime) {
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || WallNormal.Size() < 0.1f)
		return;

	FVector CurrentForward = ControlledPawn->GetActorForwardVector();
	FVector CrossProduct = FVector::CrossProduct(WallNormal, FVector::UpVector);

	FVector AvoidanceDirection;
	if (FVector::DotProduct(CrossProduct, CurrentForward) > 0.0f)
		AvoidanceDirection = CrossProduct;
	else
		AvoidanceDirection = -CrossProduct;

	FRotator CurrentRotation = ControlledPawn->GetActorRotation();
	FRotator TargetRotation = AvoidanceDirection.Rotation();

	FRotator NewRotation = FMath::RInterpConstantTo(CurrentRotation, TargetRotation, deltaTime, AvoidanceRotationSpeed);

	ControlledPawn->SetActorRotation(NewRotation);
}


