// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ClimbingComponent.generated.h"

/**
 * 
 */
UCLASS()
class HORIZONSEDGE_API UClimbingComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	UPROPERTY(Transient) class AHorizonsEdgeCharacter* HorizonsEdgeCharacterOwner;
	
	// Climb
	UPROPERTY(EditDefaultsOnly) float MaxClimbSpeed = 300.f;
public:
	UFUNCTION(BlueprintCallable)
		void OnClicked(const FVector& ClimbPointLocation);
		
protected:
	virtual void InitializeComponent() override;

	// Helpers
private:
	float CapR() const;
	float CapHH() const;
};
