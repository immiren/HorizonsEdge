// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbingComponent.h"
#include "HorizonsEdgeCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

void UClimbingComponent::OnClicked(const FVector& ClimbPointLocation)
{
	//check for collisions betweeen player and end point. possibly useless or at least difficult actually lets leave this out for now
	//continue if no collisions/nothing inbetween

	//start and end points
	FVector TargetLocation = ClimbPointLocation + CapR() * 1.01f+ FVector::DownVector * CapHH();
	FVector CurrentLocation = UpdatedComponent->GetComponentLocation();

	SetMovementMode(MOVE_Flying); // removes gravity

	//transition player from current point to new point
	float UpSpeed = Velocity | FVector::UpVector;
	float TransDistance = FVector::Dist(TargetLocation, UpdatedComponent->GetComponentLocation());

	HorizonsEdgeCharacterOwner->SetActorLocation(TargetLocation); //teleports lol make this better once the basics work
}

void UClimbingComponent::InitializeComponent()
{
	Super::InitializeComponent();

	HorizonsEdgeCharacterOwner = Cast<AHorizonsEdgeCharacter>(GetOwner());
}

float UClimbingComponent::CapR() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float UClimbingComponent::CapHH() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}
