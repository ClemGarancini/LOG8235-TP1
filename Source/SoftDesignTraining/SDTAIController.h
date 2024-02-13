// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PhysicsHelpers.h"


#include "SDTAIController.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public AAIController
{
    GENERATED_BODY()
public:
    virtual void Tick(float deltaTime) override;
    virtual void BeginPlay() override;
private:
    //Debug
    bool DEBUG_WALL_FRONT;
    bool DEBUG_WALL_RIGHT;
    bool DEBUG_WALL_LEFT;
    // Properties
    FVector acceleration;
    float maxVelocity;
    FVector velocity;
    FRotator orientation;
    
    // Obstacle detection
    float sweepDistance;
    TArray<AActor *> floorActor;
    
    // Smooth rotation
    float rotationStartTime;
    float rotationDuration;
    FRotator startOrientation;
    FRotator endOrientation;
    bool inRotation;
    
    // Methods
    void ApplyVelocityAndOrientation(APawn * pawn);
    FVector GetVelocityVector(APawn const * pawn, float deltaTime);
    int CollectObstacles(APawn const * pawn, PhysicsHelpers& physicHelper, FHitResult& hitResultWall, FHitResult& hitResultDeathFloor);
    FRotator ChooseNewOrientation(APawn const * pawn, PhysicsHelpers& physicHelper, FVector normalImpact);
    void StartRotation(FRotator targetOrientation);
    void PerformRotation(APawn * pawn);
    bool SearchPickup(APawn * pawn, FVector & positionPickup);
    
};
