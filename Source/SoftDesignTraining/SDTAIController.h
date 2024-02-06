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
    virtual void OnPossess(APawn* InPawn) override;
private:
    // Properties
    FVector acceleration;
    float maxVelocity;
    float beginLerp;
    FVector velocity;
    FRotator orientation;
    
    // Methods
    FVector GetVelocityVector(APawn const * pawn, float deltaTime);
    FRotator GetOrientationVector();
    bool CollectObstacles(APawn const * pawn, PhysicsHelpers& physicHelper, FHitResult& hitResult);
    
};
