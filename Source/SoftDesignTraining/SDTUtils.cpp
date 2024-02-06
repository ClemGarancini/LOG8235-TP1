// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTUtils.h"
#include "SoftDesignTraining.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

/*static*/ bool SDTUtils::Raycast(UWorld* uWorld, FVector sourcePoint, FVector targetPoint)
{
    FHitResult hitData;
    FCollisionQueryParams TraceParams(FName(TEXT("VictoreCore Trace")), true);

    return uWorld->LineTraceSingleByChannel(hitData, sourcePoint, targetPoint, ECC_Pawn, TraceParams);
}

bool SDTUtils::IsPlayerPoweredUp(UWorld * uWorld)
{
    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(uWorld, 0);
    if (!playerCharacter)
        return false;

    ASoftDesignTrainingMainCharacter* castedPlayerCharacter = Cast<ASoftDesignTrainingMainCharacter>(playerCharacter);
    if (!castedPlayerCharacter)
        return false;

    return castedPlayerCharacter->IsPoweredUp();
}

FVector SDTUtils::GetNewVelocityFromAcceleration(FVector velocity, float deltaTime, FVector acceleration, float maxVelocity)
{
    // Computes the new velocity given the actual velocity of the pawn and the desired acceleration
    FVector result = FVector(
                             FMath::Clamp(velocity.X + acceleration.X * deltaTime, -maxVelocity, maxVelocity),
                             FMath::Clamp(velocity.Y + acceleration.Y * deltaTime, -maxVelocity, maxVelocity),
                             FMath::Clamp(velocity.Z + acceleration.Z * deltaTime, -maxVelocity, maxVelocity));
    return result;
}

FRotator SDTUtils::GetNewOrientationFromVelocity(FVector velocity)
{
    // Computes the new orientation of the pawn given its velocity
    FMatrix orientationMatrix = FRotationMatrix::MakeFromX(velocity);
    return orientationMatrix.Rotator();
}
