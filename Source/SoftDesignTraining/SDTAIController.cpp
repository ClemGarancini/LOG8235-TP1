// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicsHelpers.h"
#include "SDTUtils.h"
#include "Kismet/GameplayStatics.h"


void ASDTAIController::BeginPlay()
{
    Super::BeginPlay();
    // Constant
    maxVelocity = .2f;
    
    // Frame acceleration, velocity and orientation
    acceleration = FVector(0.f, 1.f, 0.f);
    velocity = FVector(0.f, 0.f, 0.f);
    orientation = FRotator(0.f, 0.f, 0.f);
    
    // Obstacle Detection
    sweepDistance = 150.f;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), "Floor", floorActor);
    
    //Smooth  Rotation
    rotationDuration = 1.f;
    // State of the agent
    inRotation = false;
}

void ASDTAIController::Tick(float deltaTime)
{
    Super::Tick(deltaTime);
    
    // Useful variables
    UWorld * world = GetWorld();
    PhysicsHelpers physicsHelpers(world);
    APawn* pawn = GetPawn();
    
    // Prevent crash if pawn is not found
    if (pawn == nullptr) {
        UE_LOG(LogTemp, Log, TEXT("Pawn is null"));
        return;
    }

    // Collection of obstacles
    FHitResult hitResultWall;
    FHitResult hitResultDeathFloor;
    int iHit = CollectObstacles(pawn, physicsHelpers, hitResultWall, hitResultDeathFloor);
    
    // Search of pickup collectibles
    FVector pickupLocation;
    bool bHit = SearchPickup(pawn, pickupLocation);

    // Compute acceleration, velocity and orientation for the next frame
    if (inRotation) {
        // Agent is performing a rotation to avoid obstacles
        PerformRotation(pawn);
    } else {
        switch (iHit) {
            case 0: {
                // No obstacles detected, forward movement
                velocity = GetVelocityVector(pawn, deltaTime);
                orientation = velocity.Rotation();
                break;
            }
            case 1: {
                // A wall has been detected
                inRotation = true;
                FRotator targetOrientation = ChooseNewOrientation(pawn, physicsHelpers, hitResultWall.ImpactNormal);
                StartRotation(targetOrientation);
                break;
            }
            case 2: {
                // A DeathFloor has been detected
                inRotation = true;
                FRotator targetOrientation = ChooseNewOrientation(pawn, physicsHelpers, hitResultDeathFloor.ImpactNormal);
                StartRotation(targetOrientation);
                break;
            }
            default:
                break;
        }
    }
    
    // We apply the computed velocity and orientation
    ApplyVelocityAndOrientation(pawn);
}

void ASDTAIController::ApplyVelocityAndOrientation(APawn * pawn) // Apply the frame velocity and orientation
{
    pawn->AddMovementInput(velocity);
    pawn->SetActorRotation(orientation);
}

FVector ASDTAIController::GetVelocityVector(APawn const * pawn, float deltaTime) // Compute velocity from acceleration
{
    FVector newVelocity = SDTUtils::GetNewVelocityFromAcceleration(velocity, deltaTime, acceleration, maxVelocity);
    return newVelocity;
}

int ASDTAIController::CollectObstacles(APawn const * pawn, PhysicsHelpers& physicHelper, FHitResult& hitResultWall, FHitResult& hitResultDeathFloor) //Collect obstacles (walls and deathfloor) in front of the agent and returns 0, 1, 2 depending on the obstacles encountered
{
    // Variables for the sweep
    FVector sphereCenter = pawn->GetActorLocation();
    float sphereRadius = 50.f;
    
    FVector sweepDirection = pawn->GetActorForwardVector();
    
    FCollisionQueryParams collisionParams;
    collisionParams.AddIgnoredActor(pawn);
    // Ignore the floor for the detection of the deathfloor
    if (floorActor.Num()>0) collisionParams.AddIgnoredActor(floorActor[0]);
    
    // Do a sweep to find a wall
    bool bHitWall = GetWorld()->SweepSingleByChannel(
            hitResultWall,
            sphereCenter,
            sphereCenter + sweepDirection * sweepDistance,
            FQuat::Identity,
            ECC_Visibility,
            FCollisionShape::MakeSphere(sphereRadius),
            collisionParams
        );
    
    FVector sphereCenterDeathFloor = FVector(sphereCenter.X, sphereCenter.Y, 140.f);
    
    // Do a sweep to find a death floor
    bool bHitDeathFloor = GetWorld()->SweepSingleByChannel(
            hitResultDeathFloor,
            sphereCenterDeathFloor,
            sphereCenterDeathFloor + sweepDirection * sweepDistance,
            FQuat::Identity,
            ECC_GameTraceChannel2,  // ou utilisez le canal approprié pour vos besoins
            FCollisionShape::MakeSphere(sphereRadius),
            collisionParams
        );
    
    return bHitWall ? 1 : (bHitDeathFloor ? 2 : 0);
}

FRotator ASDTAIController::ChooseNewOrientation(APawn const * pawn, PhysicsHelpers& physicHelper, FVector normalImpact) // Choose a new orientation to take when an obstacle is encountered
{
    FHitResult hitRight;
    FHitResult hitLeft;
    FVector sphereCenter = pawn->GetActorLocation();
    float sphereRadius = 50.f;
    
    // Check at the right and the left of the agent
    FVector sweepDirectionRight = FVector::CrossProduct(normalImpact, FVector(0.f,0.f,1.f));
    FVector sweepDirectionLeft = -sweepDirectionRight;
    
    FCollisionQueryParams collisionParams;
    collisionParams.AddIgnoredActor(pawn);
    
    bool bHitRight = GetWorld()->SweepSingleByChannel(
            hitRight,
            sphereCenter,
            sphereCenter + sweepDirectionRight * sweepDistance,
            FQuat::Identity,
            ECC_Visibility,
            FCollisionShape::MakeSphere(sphereRadius),
            collisionParams
        );
    
    bool bHitLeft = GetWorld()->SweepSingleByChannel(
            hitLeft,
            sphereCenter,
            sphereCenter + sweepDirectionLeft * sweepDistance,
            FQuat::Identity,
            ECC_Visibility,
            FCollisionShape::MakeSphere(sphereRadius),
            collisionParams
        );
    
    // It choose by the first available direction between the right, the left and the back
    return bHitRight ? (bHitLeft ? normalImpact.Rotation() : sweepDirectionLeft.Rotation()) : sweepDirectionRight.Rotation();
}

void ASDTAIController::StartRotation(FRotator targetOrientation) // Begin the rotation to avoid an object
{
    rotationStartTime = GetWorld()->GetTimeSeconds();
    startOrientation = orientation;
    endOrientation = targetOrientation;
}

void ASDTAIController::PerformRotation(APawn * pawn) // Perform the rotation by interpotaling between the intial one and the desired one
{
    float elapsedTime = GetWorld()->GetTimeSeconds() - rotationStartTime;
    
    if (elapsedTime < rotationDuration)
    {
        float alpha = FMath::Clamp(elapsedTime / rotationDuration, 0.0f, 1.0f);
        FRotator newOrientation = FRotator(
                                        FMath::Lerp(startOrientation.Pitch, endOrientation.Pitch, alpha),
                                        FMath::Lerp(startOrientation.Yaw, endOrientation.Yaw, alpha),
                                        FMath::Lerp(startOrientation.Roll, endOrientation.Roll, alpha));
        orientation = newOrientation;
        velocity = maxVelocity * newOrientation.Vector();
    }
    else 
    {
        orientation = endOrientation;
        velocity = maxVelocity * endOrientation.Vector();
        
        inRotation = false;
        acceleration = velocity;
        acceleration.Normalize();
    }
}

bool ASDTAIController::SearchPickup(APawn * pawn, FVector & positionPickup)
{
    FHitResult hitResult;
    float j;
    float angleRaycast;
    FQuat rotationQuat;
    bool bHit;
    FVector forwardVector = pawn->GetActorForwardVector();
    FVector endLocation;
    FCollisionQueryParams collisionParams;
    
    collisionParams.AddIgnoredActor(pawn);

    bHit = GetWorld()->LineTraceSingleByChannel(hitResult,
                                             pawn->GetActorLocation() + FVector(0.f,0.f,45.f),
                                             pawn->GetActorLocation() + 300.f * forwardVector + FVector(0.f,0.f,45.f),
                                             ECC_Visibility,
                                             collisionParams
                                            );
    DrawDebugLine(GetWorld(),
                  pawn->GetActorLocation() + FVector(0.f,0.f,45.f),
                  pawn->GetActorLocation() + 300.f * forwardVector + FVector(0.f,0.f,45.f),
                  FColor::Green,
                  false,
                  -1,
                  0,
                  0);
    if (bHit) {
        FString s = hitResult.Component->GetOwner()->GetName();
        UE_LOG(LogTemp, Log, TEXT("found %s"),*s);
    }
    
    for (int i = 0; i < 20; i++) {
        j = i;
        angleRaycast = -30.f + j/19.f * 60.f;
        
        rotationQuat = FQuat::MakeFromEuler(FVector(0.0f, 0.0f, angleRaycast));
        endLocation = pawn->GetActorLocation() + 300.f * rotationQuat.RotateVector(forwardVector);


        bHit = GetWorld()->LineTraceSingleByChannel(hitResult,
                                                 pawn->GetActorLocation() + FVector(0.f,0.f,45.f),
                                                 endLocation + FVector(0.f,0.f,45.f),
                                                 ECC_Visibility,
                                                 collisionParams
                                                );
        
        DrawDebugLine(GetWorld(),
                      pawn->GetActorLocation(),
                      endLocation,
                      FColor::Green,
                      false,
                      -1,
                      0,
                      0);
        if (bHit) {
            if (hitResult.Component.Get()->ComponentHasTag("Pickup")) {
                positionPickup = hitResult.Location;
                return true;
            }
        }
    }
    return false;
}





