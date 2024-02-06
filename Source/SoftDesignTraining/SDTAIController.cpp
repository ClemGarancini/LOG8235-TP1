// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicsHelpers.h"
#include "SDTUtils.h"


void ASDTAIController::OnPossess(APawn* InPawn)
{
    acceleration = FVector(0.1f, 0.f, 0.f);
    maxVelocity = 0.2f;
    beginLerp = 0.5f;
    velocity = FVector(0.f, 0.f, 0.f);
    orientation = FRotator(0.f, 0.f, 0.f);
    
    //DEBUG
    UE_LOG(LogTemp, Log, TEXT("Init"));
}

void ASDTAIController::Tick(float deltaTime)
{
    FString s1 = this->acceleration.ToString();
//    UE_LOG(LogTemp, Log, TEXT("Acc: %s"), *s1);
    
    // Useful variables
    static UWorld * World = GetWorld();
    static APawn* pawn = GetPawn();
    static PhysicsHelpers newPhysicHelper(World);
    
    // Q1: Compute and add velocity and rotation to the pawn
    this->velocity = GetVelocityVector(pawn, deltaTime);
    this->orientation = GetOrientationVector();
    
    
    // Q2: Wall detection and avoid navigation
    FHitResult hitResult;
    bool bHit = CollectObstacles(pawn, newPhysicHelper, hitResult);
    
    
    // Apply velocity and orientation
    pawn->AddMovementInput(this->velocity);
    pawn->SetActorRotation(this->orientation);
    
    
    //#################### DEBUG ####################
    if (bHit){
        FVector normalHit = hitResult.ImpactNormal;
        FString s = normalHit.ToString();
        UE_LOG(LogTemp, Log, TEXT("%s"), *s);
    }
    
//    if (obstaclesDetected.Num() != 0){
//        UE_LOG(LogTemp, Log, TEXT("Obstacle detected"));
//    }
    
    
    // Print and debug
    // FVector pawnLocation = pawn->GetActorLocation();
//    FString s = velocity.ToString();
//    UE_LOG(LogTemp, Log, TEXT("Vel: %s"), *s);
    
}

FVector ASDTAIController::GetVelocityVector(APawn const * pawn, float deltaTime)
{
    FVector pawnVelocity = pawn->GetVelocity();
    FVector newVelocity = SDTUtils::GetNewVelocityFromAcceleration(pawnVelocity, deltaTime, this->acceleration, this->maxVelocity);
    
    return newVelocity;
}

FRotator ASDTAIController::GetOrientationVector()
{
    return SDTUtils::GetNewOrientationFromVelocity(this->velocity);
}

bool ASDTAIController::CollectObstacles(APawn const * pawn, PhysicsHelpers& physicHelper, FHitResult& hitResult)
{
    
    FVector sphereCenter = pawn->GetActorLocation();
    float sphereRadius = 50.f;
    
    FVector sweepDirection = pawn->GetActorForwardVector();
    float sweepDistance = 200.f;
    
    FCollisionQueryParams collisionParams;
    collisionParams.AddIgnoredActor(pawn);
    
    bool bHit = GetWorld()->SweepSingleByChannel(
            hitResult,
            sphereCenter,
            sphereCenter + sweepDirection * sweepDistance,
            FQuat::Identity,
            ECC_Visibility,  // ou utilisez le canal approprié pour vos besoins
            FCollisionShape::MakeSphere(sphereRadius),
            collisionParams
        );
    
    return bHit;
}

//FVector ASDTAIController::GetCorrectedVelocity(APawn const * pawn, FHitResult hitResult){
//    
//}





