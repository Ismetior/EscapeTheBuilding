// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Grabber.h"
#include "DrawDebugHelpers.h"

#define OUT

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();
	
	FindPhysicsHandle();

	SetupInputComponent();
}

// Checking for Input Component
void UGrabber::SetupInputComponent() {
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();
	if (InputComponent) {
		InputComponent->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);
		InputComponent->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	}
}

// Checking for PhysicsHandle Component
void UGrabber::FindPhysicsHandle() {
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle  == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("The Physics Object %s is not having the PhysicsHandle Component Attatched to it..."), *GetOwner()->GetName());
	}
}

// if we hit something then attatch the physics handle.
void UGrabber::Grab() {
	FHitResult HitResult = GetFirstPhysicsBodyInReach();
	UPrimitiveComponent* ComponentToGrab = HitResult.GetComponent();
	
	if (HitResult.GetActor()) {
		if (!PhysicsHandle) {return;}
		PhysicsHandle->GrabComponentAtLocation(
			ComponentToGrab,
			NAME_None,
			GetPlayersReach()
		);
	}
}

void UGrabber::Release() {
	if (!PhysicsHandle) {return;}
	PhysicsHandle->ReleaseComponent();
}

// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// if the physics handle is attatched
	if (!PhysicsHandle) {return;}
	if (PhysicsHandle->GrabbedComponent) {
		//  then move the object we are holding
		PhysicsHandle->SetTargetLocation(GetPlayersReach());
	}
}

// Ray-cast out to a certain distance(Reach)
FHitResult UGrabber::GetFirstPhysicsBodyInReach() const {
	FHitResult Hit;
	FCollisionQueryParams TraceParams(TEXT(""), false, GetOwner());
	
	GetWorld()->LineTraceSingleByObjectType(
		Hit,
		GetPlayersWorldLocation(),
		GetPlayersReach(),
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParams
	);
	
	return Hit;
}

FVector UGrabber::GetPlayersWorldLocation() const {
	FVector PlayerViewPointLocation = GetOwner()->GetActorLocation();
	FRotator PlayerViewPointRotation = GetOwner()->GetActorRotation();

	return PlayerViewPointLocation;
}

FVector UGrabber::GetPlayersReach() const {
	FVector PlayerViewPointLocation = GetOwner()->GetActorLocation();
	FRotator PlayerViewPointRotation = GetOwner()->GetActorRotation();

	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);

	return PlayerViewPointLocation + PlayerViewPointRotation.Vector() * Reach;	
}

