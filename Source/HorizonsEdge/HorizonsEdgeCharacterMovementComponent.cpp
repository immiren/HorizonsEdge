// Fill out your copyright notice in the Description page of Project Settings.


#include "HorizonsEdgeCharacterMovementComponent.h"
#include "HorizonsEdgeCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

// Helper Macros
#if 0
float MacroDuration = 2.f;
#define SLOG(x) GEngine->AddOnScreenDebugMessage(-1, MacroDuration ? MacroDuration : -1.f, FColor::Yellow, x);
#define POINT(x, c) DrawDebugPoint(GetWorld(), x, 10, c, !MacroDuration, MacroDuration);
#define LINE(x1, x2, c) DrawDebugLine(GetWorld(), x1, x2, c, !MacroDuration, MacroDuration);
#define CAPSULE(x, c) DrawDebugCapsule(GetWorld(), x, CapHH(), CapR(), FQuat::Identity, c, !MacroDuration, MacroDuration);
#else
#define SLOG(x)
#define POINT(x, c)
#define LINE(x1, x2, c)
#define CAPSULE(x, c)
#endif



#pragma region Saved Move

UHorizonsEdgeCharacterMovementComponent::FSavedMove_HorizonsEdge::FSavedMove_HorizonsEdge()
{
	Saved_bPrevWantsToCrouch = 0;
}

bool UHorizonsEdgeCharacterMovementComponent::FSavedMove_HorizonsEdge::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_HorizonsEdge* NewHorizonsEdgeMove = static_cast<FSavedMove_HorizonsEdge*>(NewMove.Get());

	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UHorizonsEdgeCharacterMovementComponent::FSavedMove_HorizonsEdge::Clear()
{
	FSavedMove_Character::Clear();

	Saved_bPressedJump = 0;

	Saved_bHadAnimRootMotion = 0;
	Saved_bTransitionFinished = 0;

	Saved_bPrevWantsToCrouch = 0;
}

uint8 UHorizonsEdgeCharacterMovementComponent::FSavedMove_HorizonsEdge::GetCompressedFlags() const
{
	uint8 Result = FSavedMove_Character::GetCompressedFlags();

	if (Saved_bPressedJump) Result |= FLAG_JumpPressed;

	return Result;
}

void UHorizonsEdgeCharacterMovementComponent::FSavedMove_HorizonsEdge::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	const UHorizonsEdgeCharacterMovementComponent* CharacterMovement = Cast<UHorizonsEdgeCharacterMovementComponent>(C->GetCharacterMovement());

	Saved_bPrevWantsToCrouch = CharacterMovement->Safe_bPrevWantsToCrouch;
	Saved_bPressedJump = CharacterMovement->HorizonsEdgeCharacterOwner->bPressedJump;

	Saved_bHadAnimRootMotion = CharacterMovement->Safe_bHadAnimRootMotion;
	Saved_bTransitionFinished = CharacterMovement->Safe_bTransitionFinished;
}

void UHorizonsEdgeCharacterMovementComponent::FSavedMove_HorizonsEdge::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);

	UHorizonsEdgeCharacterMovementComponent* CharacterMovement = Cast<UHorizonsEdgeCharacterMovementComponent>(C->GetCharacterMovement());

	CharacterMovement->Safe_bPrevWantsToCrouch = Saved_bPrevWantsToCrouch;
	CharacterMovement->HorizonsEdgeCharacterOwner->bPressedJump = Saved_bPressedJump;

	CharacterMovement->Safe_bHadAnimRootMotion = Saved_bHadAnimRootMotion;
	CharacterMovement->Safe_bTransitionFinished = Saved_bTransitionFinished;
}

#pragma endregion

#pragma region Client Network Prediction Data

UHorizonsEdgeCharacterMovementComponent::FNetworkPredictionData_Client_HorizonsEdge::FNetworkPredictionData_Client_HorizonsEdge(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UHorizonsEdgeCharacterMovementComponent::FNetworkPredictionData_Client_HorizonsEdge::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_HorizonsEdge());
}

#pragma endregion

UHorizonsEdgeCharacterMovementComponent::UHorizonsEdgeCharacterMovementComponent()
{
	NavAgentProps.bCanCrouch = true;
	HorizonsEdgeServerMoveBitWriter.SetAllowResize(true);
}

#pragma region CMC

void UHorizonsEdgeCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	HorizonsEdgeCharacterOwner = Cast<AHorizonsEdgeCharacter>(GetOwner());
}

// Network
void UHorizonsEdgeCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
}

void UHorizonsEdgeCharacterMovementComponent::OnClientCorrectionReceived(FNetworkPredictionData_Client_Character& ClientData,
	float TimeStamp, FVector NewLocation, FVector NewVelocity, UPrimitiveComponent* NewBase, FName NewBaseBoneName,
	bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode)
{
	Super::OnClientCorrectionReceived(ClientData, TimeStamp, NewLocation, NewVelocity, NewBase, NewBaseBoneName,
		bHasBase, bBaseRelativePosition,
		ServerMovementMode);

	CorrectionCount++;
}


FNetworkPredictionData_Client* UHorizonsEdgeCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)

		if (ClientPredictionData == nullptr)
		{
			UHorizonsEdgeCharacterMovementComponent* MutableThis = const_cast<UHorizonsEdgeCharacterMovementComponent*>(this);

			MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_HorizonsEdge(*this);
			MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
			MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
		}
	return ClientPredictionData;
}
// Getters / Helpers
bool UHorizonsEdgeCharacterMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround();
}
bool UHorizonsEdgeCharacterMovementComponent::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}
float UHorizonsEdgeCharacterMovementComponent::GetMaxSpeed() const
{
	if (MovementMode != MOVE_Custom) return Super::GetMaxSpeed();

	switch (CustomMovementMode)
	{
	case CMOVE_Hang:
		return 0.f;
	case CMOVE_Climb:
		return MaxClimbSpeed;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
			return -1.f;
	}
}
float UHorizonsEdgeCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if (MovementMode != MOVE_Custom) return Super::GetMaxBrakingDeceleration();

	switch (CustomMovementMode)
	{
	case CMOVE_Hang:
		return 0.f;
	case CMOVE_Climb:
		return BrakingDecelerationClimbing;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
			return -1.f;
	}
}

bool UHorizonsEdgeCharacterMovementComponent::CanAttemptJump() const
{
	return Super::CanAttemptJump() || IsHanging() || IsClimbing();
}

bool UHorizonsEdgeCharacterMovementComponent::DoJump(bool bReplayingMoves)
{
	bool bWasOnWall = IsHanging() || IsClimbing();
	if (Super::DoJump(bReplayingMoves))
	{
		if (bWasOnWall)
		{
			if (!bReplayingMoves)
			{
				CharacterOwner->PlayAnimMontage(WallJumpMontage);
			}
			Velocity += FVector::UpVector * WallJumpForce * .5f;
			Velocity += Acceleration.GetSafeNormal2D() * WallJumpForce * .5f;
		}
		return true;
	}
	return false;
}



// Movement Pipeline
void UHorizonsEdgeCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if (IsFalling() && bWantsToCrouch)
	{
		if (TryClimb()) bWantsToCrouch = false;
	}
	else if ((IsClimbing() || IsHanging()) && bWantsToCrouch)
	{
		SetMovementMode(MOVE_Falling);
		bWantsToCrouch = false;
	}

	// Try Mantle
	if (HorizonsEdgeCharacterOwner->bPressedJump)
	{
		SLOG("Trying HorizonsEdgejump")
			if (TryMantle())
			{
				HorizonsEdgeCharacterOwner->StopJumping();
			}
			else if (TryHang())
			{

				HorizonsEdgeCharacterOwner->StopJumping();
			}
			else
			{
				SLOG("Failed Mantle, Reverting to jump")
					HorizonsEdgeCharacterOwner->bPressedJump = false;
				CharacterOwner->bPressedJump = true;
				CharacterOwner->CheckJumpInput(DeltaSeconds);
				bOrientRotationToMovement = true;

			}

	}

	// Transition
	if (Safe_bTransitionFinished)
	{
		SLOG("Transition Finished")
			UE_LOG(LogTemp, Warning, TEXT("FINISHED RM"))

			if (TransitionName == "Mantle")
			{
				if (IsValid(TransitionQueuedMontage))
				{
					SetMovementMode(MOVE_Flying);
					CharacterOwner->PlayAnimMontage(TransitionQueuedMontage, TransitionQueuedMontageSpeed);
					TransitionQueuedMontageSpeed = 0.f;
					TransitionQueuedMontage = nullptr;
				}
				else
				{
					SetMovementMode(MOVE_Walking);
				}
			}
			else if (TransitionName == "Hang")
			{
				SetMovementMode(MOVE_Custom, CMOVE_Hang);
				Velocity = FVector::ZeroVector;
			}

		TransitionName = "";
		Safe_bTransitionFinished = false;
	}

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UHorizonsEdgeCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);

	if (!HasAnimRootMotion() && Safe_bHadAnimRootMotion && IsMovementMode(MOVE_Flying))
	{
		UE_LOG(LogTemp, Warning, TEXT("Ending Anim Root Motion"))
			SetMovementMode(MOVE_Walking);
	}

	if (GetRootMotionSourceByID(TransitionRMS_ID) && GetRootMotionSourceByID(TransitionRMS_ID)->Status.HasFlag(ERootMotionSourceStatusFlags::Finished))
	{
		RemoveRootMotionSourceByID(TransitionRMS_ID);
		Safe_bTransitionFinished = true;
	}


	Safe_bHadAnimRootMotion = HasAnimRootMotion();
}

void UHorizonsEdgeCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CMOVE_Hang:
		break;
	case CMOVE_Climb:
		PhysClimb(deltaTime, Iterations);
		break;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
	}
}
void UHorizonsEdgeCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	Safe_bPrevWantsToCrouch = bWantsToCrouch;
}

// Movement Event
void UHorizonsEdgeCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (IsFalling())
	{
		bOrientRotationToMovement = true;
	}
}

bool UHorizonsEdgeCharacterMovementComponent::ServerCheckClientError(float ClientTimeStamp, float DeltaTime,
	const FVector& Accel, const FVector& ClientWorldLocation, const FVector& RelativeClientLocation,
	UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	if (GetCurrentNetworkMoveData()->NetworkMoveType == FCharacterNetworkMoveData::ENetworkMoveType::NewMove)
	{
		float LocationError = FVector::Dist(UpdatedComponent->GetComponentLocation(), ClientWorldLocation);
		GEngine->AddOnScreenDebugMessage(6, 100.f, FColor::Yellow, FString::Printf(TEXT("Loc: %s"), *ClientWorldLocation.ToString()));
		AccumulatedClientLocationError += LocationError * DeltaTime;
	}



	return Super::ServerCheckClientError(ClientTimeStamp, DeltaTime, Accel, ClientWorldLocation, RelativeClientLocation,
		ClientMovementBase,
		ClientBaseBoneName, ClientMovementMode);

}


void UHorizonsEdgeCharacterMovementComponent::CallServerMovePacked(const FSavedMove_Character* NewMove, const FSavedMove_Character* PendingMove, const FSavedMove_Character* OldMove)
{
	// Get storage container we'll be using and fill it with movement data
	FCharacterNetworkMoveDataContainer& MoveDataContainer = GetNetworkMoveDataContainer();
	MoveDataContainer.ClientFillNetworkMoveData(NewMove, PendingMove, OldMove);

	// Reset bit writer without affecting allocations
	FBitWriterMark BitWriterReset;
	BitWriterReset.Pop(HorizonsEdgeServerMoveBitWriter);

	// 'static' to avoid reallocation each invocation
	static FCharacterServerMovePackedBits PackedBits;
	UNetConnection* NetConnection = CharacterOwner->GetNetConnection();


	{
		// Extract the net package map used for serializing object references.
		HorizonsEdgeServerMoveBitWriter.PackageMap = NetConnection ? ToRawPtr(NetConnection->PackageMap) : nullptr;
	}

	if (HorizonsEdgeServerMoveBitWriter.PackageMap == nullptr)
	{
		UE_LOG(LogNetPlayerMovement, Error, TEXT("CallServerMovePacked: Failed to find a NetConnection/PackageMap for data serialization!"));
		return;
	}

	// Serialize move struct into a bit stream
	if (!MoveDataContainer.Serialize(*this, HorizonsEdgeServerMoveBitWriter, HorizonsEdgeServerMoveBitWriter.PackageMap) || HorizonsEdgeServerMoveBitWriter.IsError())
	{
		UE_LOG(LogNetPlayerMovement, Error, TEXT("CallServerMovePacked: Failed to serialize out movement data!"));
		return;
	}

	// Copy bits to our struct that we can NetSerialize to the server.
	PackedBits.DataBits.SetNumUninitialized(HorizonsEdgeServerMoveBitWriter.GetNumBits());

	check(PackedBits.DataBits.Num() >= HorizonsEdgeServerMoveBitWriter.GetNumBits());
	FMemory::Memcpy(PackedBits.DataBits.GetData(), HorizonsEdgeServerMoveBitWriter.GetData(), HorizonsEdgeServerMoveBitWriter.GetNumBytes());

	TotalBitsSent += PackedBits.DataBits.Num();

	// Send bits to server!
	ServerMovePacked_ClientSend(PackedBits);

	MarkForClientCameraUpdate();
}

#pragma endregion

#pragma region Mantle

bool UHorizonsEdgeCharacterMovementComponent::TryMantle()
{
	if (!(IsMovementMode(MOVE_Walking) && !IsCrouching()) && !IsMovementMode(MOVE_Falling)) return false;

	// Helper Variables
	FVector BaseLoc = UpdatedComponent->GetComponentLocation() + FVector::DownVector * CapHH();
	FVector Fwd = UpdatedComponent->GetForwardVector().GetSafeNormal2D();
	auto Params = HorizonsEdgeCharacterOwner->GetIgnoreCharacterParams();
	float MaxHeight = CapHH() * 2 + MantleReachHeight;
	float CosMMWSA = FMath::Cos(FMath::DegreesToRadians(MantleMinWallSteepnessAngle));
	float CosMMSA = FMath::Cos(FMath::DegreesToRadians(MantleMaxSurfaceAngle));
	float CosMMAA = FMath::Cos(FMath::DegreesToRadians(MantleMaxAlignmentAngle));


	SLOG("Starting Mantle Attempt")

		// Check Front Face
		FHitResult FrontHit;
	float CheckDistance = FMath::Clamp(Velocity | Fwd, CapR() + 30, MantleMaxDistance);
	FVector FrontStart = BaseLoc + FVector::UpVector * (MaxStepHeight - 1);
	for (int i = 0; i < 6; i++)
	{
		LINE(FrontStart, FrontStart + Fwd * CheckDistance, FColor::Red)
			if (GetWorld()->LineTraceSingleByProfile(FrontHit, FrontStart, FrontStart + Fwd * CheckDistance, "BlockAll", Params)) break;
		FrontStart += FVector::UpVector * (2.f * CapHH() - (MaxStepHeight - 1)) / 5;
	}
	if (!FrontHit.IsValidBlockingHit()) return false;
	float CosWallSteepnessAngle = FrontHit.Normal | FVector::UpVector;
	if (FMath::Abs(CosWallSteepnessAngle) > CosMMWSA || (Fwd | -FrontHit.Normal) < CosMMAA) return false;

	POINT(FrontHit.Location, FColor::Red);

	// Check Height
	TArray<FHitResult> HeightHits;
	FHitResult SurfaceHit;
	FVector WallUp = FVector::VectorPlaneProject(FVector::UpVector, FrontHit.Normal).GetSafeNormal();
	float WallCos = FVector::UpVector | FrontHit.Normal;
	float WallSin = FMath::Sqrt(1 - WallCos * WallCos);
	FVector TraceStart = FrontHit.Location + Fwd + WallUp * (MaxHeight - (MaxStepHeight - 1)) / WallSin;
	LINE(TraceStart, FrontHit.Location + Fwd, FColor::Orange)
		if (!GetWorld()->LineTraceMultiByProfile(HeightHits, TraceStart, FrontHit.Location + Fwd, "BlockAll", Params)) return false;
	for (const FHitResult& Hit : HeightHits)
	{
		if (Hit.IsValidBlockingHit())
		{
			SurfaceHit = Hit;
			break;
		}
	}
	if (!SurfaceHit.IsValidBlockingHit() || (SurfaceHit.Normal | FVector::UpVector) < CosMMSA) return false;
	float Height = (SurfaceHit.Location - BaseLoc) | FVector::UpVector;

	SLOG(FString::Printf(TEXT("Height: %f"), Height))
		POINT(SurfaceHit.Location, FColor::Blue);

	if (Height > MaxHeight) return false;


	// Check Clearance
	float SurfaceCos = FVector::UpVector | SurfaceHit.Normal;
	float SurfaceSin = FMath::Sqrt(1 - SurfaceCos * SurfaceCos);
	FVector ClearCapLoc = SurfaceHit.Location + Fwd * CapR() + FVector::UpVector * (CapHH() + 1 + CapR() * 2 * SurfaceSin);
	FCollisionShape CapShape = FCollisionShape::MakeCapsule(CapR(), CapHH());
	if (GetWorld()->OverlapAnyTestByProfile(ClearCapLoc, FQuat::Identity, "BlockAll", CapShape, Params))
	{
		CAPSULE(ClearCapLoc, FColor::Red)
			return false;
	}
	else
	{
		CAPSULE(ClearCapLoc, FColor::Green)
	}
	SLOG("Can Mantle")

		// Mantle Selection
		FVector ShortMantleTarget = GetMantleStartLocation(FrontHit, SurfaceHit, false);
	FVector TallMantleTarget = GetMantleStartLocation(FrontHit, SurfaceHit, true);

	bool bTallMantle = false;
	if (IsMovementMode(MOVE_Walking) && Height > CapHH() * 2)
		bTallMantle = true;
	else if (IsMovementMode(MOVE_Falling) && (Velocity | FVector::UpVector) < 0)
	{
		if (!GetWorld()->OverlapAnyTestByProfile(TallMantleTarget, FQuat::Identity, "BlockAll", CapShape, Params))
			bTallMantle = true;
	}
	FVector TransitionTarget = bTallMantle ? TallMantleTarget : ShortMantleTarget;
	CAPSULE(TransitionTarget, FColor::Yellow)

		// Perform Transition to Mantle
		CAPSULE(UpdatedComponent->GetComponentLocation(), FColor::Red)

		float UpSpeed = Velocity | FVector::UpVector;
	float TransDistance = FVector::Dist(TransitionTarget, UpdatedComponent->GetComponentLocation());

	TransitionQueuedMontageSpeed = FMath::GetMappedRangeValueClamped(FVector2D(-500, 750), FVector2D(.9f, 1.2f), UpSpeed);
	TransitionRMS.Reset();
	TransitionRMS = MakeShared<FRootMotionSource_MoveToForce>();
	TransitionRMS->AccumulateMode = ERootMotionAccumulateMode::Override;

	TransitionRMS->Duration = FMath::Clamp(TransDistance / 500.f, .1f, .25f);
	SLOG(FString::Printf(TEXT("Duration: %f"), TransitionRMS->Duration))
		TransitionRMS->StartLocation = UpdatedComponent->GetComponentLocation();
	TransitionRMS->TargetLocation = TransitionTarget;

	// Apply Transition Root Motion Source
	Velocity = FVector::ZeroVector;
	SetMovementMode(MOVE_Flying);
	TransitionRMS_ID = ApplyRootMotionSource(TransitionRMS);
	TransitionName = "Mantle";

	// Animations
	if (bTallMantle)
	{
		TransitionQueuedMontage = TallMantleMontage;
		CharacterOwner->PlayAnimMontage(TransitionTallMantleMontage, 1 / TransitionRMS->Duration);
		if (IsServer()) Proxy_bTallMantle = !Proxy_bTallMantle;
	}
	else
	{
		TransitionQueuedMontage = ShortMantleMontage;
		CharacterOwner->PlayAnimMontage(TransitionShortMantleMontage, 1 / TransitionRMS->Duration);
		if (IsServer()) Proxy_bShortMantle = !Proxy_bShortMantle;
	}

	return true;
}

FVector UHorizonsEdgeCharacterMovementComponent::GetMantleStartLocation(FHitResult FrontHit, FHitResult SurfaceHit, bool bTallMantle) const
{
	float CosWallSteepnessAngle = FrontHit.Normal | FVector::UpVector;
	float DownDistance = bTallMantle ? CapHH() * 2.f : MaxStepHeight - 1;
	FVector EdgeTangent = FVector::CrossProduct(SurfaceHit.Normal, FrontHit.Normal).GetSafeNormal();

	FVector MantleStart = SurfaceHit.Location;
	MantleStart += FrontHit.Normal.GetSafeNormal2D() * (2.f + CapR());
	MantleStart += UpdatedComponent->GetForwardVector().GetSafeNormal2D().ProjectOnTo(EdgeTangent) * CapR() * .3f;
	MantleStart += FVector::UpVector * CapHH();
	MantleStart += FVector::DownVector * DownDistance;
	MantleStart += FrontHit.Normal.GetSafeNormal2D() * CosWallSteepnessAngle * DownDistance;

	return MantleStart;
}

#pragma endregion



bool UHorizonsEdgeCharacterMovementComponent::TryHang()
{
	if (!IsMovementMode(MOVE_Falling)) return false;


	FHitResult WallHit;
	if (!GetWorld()->LineTraceSingleByProfile(WallHit, UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentLocation() + UpdatedComponent->GetForwardVector() * 300, "BlockAll", HorizonsEdgeCharacterOwner->GetIgnoreCharacterParams()))
		return false;

	TArray<FOverlapResult> OverlapResults;

	FVector ColLoc = UpdatedComponent->GetComponentLocation() + FVector::UpVector * CapHH() + UpdatedComponent->GetForwardVector() * CapR() * 3;
	auto ColBox = FCollisionShape::MakeBox(FVector(100, 100, 50));
	FQuat ColRot = FRotationMatrix::MakeFromXZ(WallHit.Normal, FVector::UpVector).ToQuat();

	if (!GetWorld()->OverlapMultiByChannel(OverlapResults, ColLoc, ColRot, ECC_WorldStatic, ColBox, HorizonsEdgeCharacterOwner->GetIgnoreCharacterParams()))
		return false;

	AActor* ClimbPoint = nullptr;

	float MaxHeight = -1e20;
	for (FOverlapResult Result : OverlapResults)
	{
		if (Result.GetActor()->ActorHasTag("Climb Point"))
		{
			float Height = Result.GetActor()->GetActorLocation().Z;
			if (Height > MaxHeight)
			{
				MaxHeight = Height;
				ClimbPoint = Result.GetActor();
			}
		}
	}
	if (!IsValid(ClimbPoint)) return false;

	FVector TargetLocation = ClimbPoint->GetActorLocation() + WallHit.Normal * CapR() * 1.01f + FVector::DownVector * CapHH();
	FQuat TargetRotation = FRotationMatrix::MakeFromXZ(-WallHit.Normal, FVector::UpVector).ToQuat();


	// Test if character can reach goal
	FTransform CurrentTransform = UpdatedComponent->GetComponentTransform();
	FHitResult Hit, ReturnHit;
	SafeMoveUpdatedComponent(TargetLocation - UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentQuat(), true, Hit);
	FVector ResultLocation = UpdatedComponent->GetComponentLocation();
	SafeMoveUpdatedComponent(CurrentTransform.GetLocation() - ResultLocation, TargetRotation, false, ReturnHit);
	if (!ResultLocation.Equals(TargetLocation)) return false;

	// Passed all conditions

	bOrientRotationToMovement = false;

	// Perform Transition to Climb Point
	float UpSpeed = Velocity | FVector::UpVector;
	float TransDistance = FVector::Dist(TargetLocation, UpdatedComponent->GetComponentLocation());

	TransitionQueuedMontageSpeed = FMath::GetMappedRangeValueClamped(FVector2D(-500, 750), FVector2D(.9f, 1.2f), UpSpeed);
	TransitionRMS.Reset();
	TransitionRMS = MakeShared<FRootMotionSource_MoveToForce>();
	TransitionRMS->AccumulateMode = ERootMotionAccumulateMode::Override;

	TransitionRMS->Duration = FMath::Clamp(TransDistance / 500.f, .1f, .25f);
	TransitionRMS->StartLocation = UpdatedComponent->GetComponentLocation();
	TransitionRMS->TargetLocation = TargetLocation;

	// Apply Transition Root Motion Source
	Velocity = FVector::ZeroVector;
	SetMovementMode(MOVE_Flying);
	TransitionRMS_ID = ApplyRootMotionSource(TransitionRMS);

	// Animations
	TransitionQueuedMontage = nullptr;
	TransitionName = "Hang";
	CharacterOwner->PlayAnimMontage(TransitionHangMontage, 1 / TransitionRMS->Duration);

	return true;
}

bool UHorizonsEdgeCharacterMovementComponent::TryClimb()
{
	if (!IsFalling()) return false;

	FHitResult SurfHit;
	FVector CapLoc = UpdatedComponent->GetComponentLocation();
	GetWorld()->LineTraceSingleByProfile(SurfHit, CapLoc, CapLoc + UpdatedComponent->GetForwardVector() * ClimbReachDistance, "BlockAll", HorizonsEdgeCharacterOwner->GetIgnoreCharacterParams());

	if (!SurfHit.IsValidBlockingHit()) return false;

	FQuat NewRotation = FRotationMatrix::MakeFromXZ(-SurfHit.Normal, FVector::UpVector).ToQuat();
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, false, SurfHit);

	SetMovementMode(MOVE_Custom, CMOVE_Climb);

	bOrientRotationToMovement = false;

	return true;
}

void UHorizonsEdgeCharacterMovementComponent::PhysClimb(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}
	if (!CharacterOwner || (!CharacterOwner->Controller && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	// Perform the move
	bJustTeleported = false;
	Iterations++;
	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FHitResult SurfHit, FloorHit;
	GetWorld()->LineTraceSingleByProfile(SurfHit, OldLocation, OldLocation + UpdatedComponent->GetForwardVector() * ClimbReachDistance, "BlockAll", HorizonsEdgeCharacterOwner->GetIgnoreCharacterParams());
	GetWorld()->LineTraceSingleByProfile(FloorHit, OldLocation, OldLocation + FVector::DownVector * CapHH() * 1.2f, "BlockAll", HorizonsEdgeCharacterOwner->GetIgnoreCharacterParams());
	if (!SurfHit.IsValidBlockingHit() || FloorHit.IsValidBlockingHit())
	{
		SetMovementMode(MOVE_Falling);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	// Transform Acceleration
	Acceleration.Z = 0.f;
	Acceleration = Acceleration.RotateAngleAxis(90.f, -UpdatedComponent->GetRightVector());

	// Apply acceleration
	CalcVelocity(deltaTime, 0.f, false, GetMaxBrakingDeceleration());
	Velocity = FVector::VectorPlaneProject(Velocity, SurfHit.Normal);

	// Compute move parameters
	const FVector Delta = deltaTime * Velocity; // dx = v * dt
	if (!Delta.IsNearlyZero())
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);
		FVector WallAttractionDelta = -SurfHit.Normal * WallAttractionForce * deltaTime;
		SafeMoveUpdatedComponent(WallAttractionDelta, UpdatedComponent->GetComponentQuat(), true, Hit);
	}

	Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime; // v = dx / dt
}


#pragma region Helpers

bool UHorizonsEdgeCharacterMovementComponent::IsServer() const
{
	return CharacterOwner->HasAuthority();
}

float UHorizonsEdgeCharacterMovementComponent::CapR() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float UHorizonsEdgeCharacterMovementComponent::CapHH() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

#pragma endregion

#pragma region Interface
void UHorizonsEdgeCharacterMovementComponent::ClimbPressed()
{
	if (IsFalling() || IsClimbing() || IsHanging()) bWantsToCrouch = true;
}

void UHorizonsEdgeCharacterMovementComponent::ClimbReleased()
{
	bWantsToCrouch = false;
}

bool UHorizonsEdgeCharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}
bool UHorizonsEdgeCharacterMovementComponent::IsMovementMode(EMovementMode InMovementMode) const
{
	return InMovementMode == MovementMode;
}

#pragma endregion

#pragma region Replication

void UHorizonsEdgeCharacterMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UHorizonsEdgeCharacterMovementComponent, Proxy_bShortMantle, COND_SkipOwner)
		DOREPLIFETIME_CONDITION(UHorizonsEdgeCharacterMovementComponent, Proxy_bTallMantle, COND_SkipOwner)
}

void UHorizonsEdgeCharacterMovementComponent::OnRep_ShortMantle()
{
	CharacterOwner->PlayAnimMontage(ProxyShortMantleMontage);
}

void UHorizonsEdgeCharacterMovementComponent::OnRep_TallMantle()
{
	CharacterOwner->PlayAnimMontage(ProxyTallMantleMontage);
}

#pragma endregion