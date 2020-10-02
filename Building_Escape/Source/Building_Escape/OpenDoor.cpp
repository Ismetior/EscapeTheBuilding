// Fill out your copyright notice in the Description page of Project Settings.

#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Components/PrimitiveComponent.h"
#include "OpenDoor.h"
#include "GameFramework/Actor.h"

#define OUT 

// Sets default values for this component's properties
UOpenDoor::UOpenDoor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UOpenDoor::BeginPlay()
{
	Super::BeginPlay();

	InitialYaw = GetOwner()->GetActorRotation().Yaw;
	CurrentYaw = InitialYaw;
	OpenAngle += InitialYaw;
	
	FindPressurePlate();
	FindAudioComponent();
}

void UOpenDoor::FindPressurePlate() {
	if (!PressurePlate) {
		UE_LOG(LogTemp, Error, TEXT("%s has OpenDoor Component on it, but no TriggeringActors set!"), *GetOwner()->GetName());
	}
}

void UOpenDoor::FindAudioComponent() {
	AudioComponent = GetOwner()->FindComponentByClass<UAudioComponent>();
	if (!AudioComponent) {
		UE_LOG(LogTemp, Error, TEXT("Audio Component not attached on %s"), *GetOwner()->GetName());
	}
}

// Called every frame
void UOpenDoor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (TotalMassOfActors() >= ThreshHoldMass) {
		OpenDoor(DeltaTime);
		DoorLastOpened = GetWorld()->GetTimeSeconds();
	}
	else {
		if (GetWorld()->GetTimeSeconds() - DoorLastOpened > DoorCloseDelay) {
			CloseDoor(DeltaTime);
		}
	}
}

void UOpenDoor::OpenDoor(float DeltaTime) {
	CurrentYaw = FMath::Lerp(CurrentYaw, OpenAngle, DeltaTime * OpenDoorSpeedMultiplier);
	FRotator DoorRotation = GetOwner()->GetActorRotation();
	DoorRotation.Yaw = CurrentYaw;
	GetOwner()->SetActorRotation(DoorRotation);

	CloseDoorSound = false;
	if (!AudioComponent) {return;}
	if (!OpenDoorSound) {
		AudioComponent->Play();
		OpenDoorSound = true;
	}
}

void UOpenDoor::CloseDoor(float DeltaTime) {
	if (GetOwner()->GetActorRotation().Yaw == InitialYaw) {return;}

	CurrentYaw = FMath::Lerp(CurrentYaw, InitialYaw, DeltaTime * CloseDoorSpeedMultiplier);
	FRotator RevertDoor = GetOwner()->GetActorRotation();
	RevertDoor.Yaw = CurrentYaw;
	GetOwner()->SetActorRotation(RevertDoor);

	OpenDoorSound = false;
	if (!AudioComponent) {return;}
	if (!CloseDoorSound) {
		AudioComponent->Play();
		CloseDoorSound = true;
	}
}

float UOpenDoor::TotalMassOfActors() const{
	float TotalMass = 0.f;

	// Find all overlapping actors
	OUT TArray<AActor*> OverlappingActors;
	if (!PressurePlate) {
		UE_LOG(LogTemp, Warning, TEXT("Pressure Plate not found on %s!"), *GetOwner()->GetName());
		return 0.f;
	}
	PressurePlate->GetOverlappingActors(OUT OverlappingActors);

	// add up their masses
	for (AActor* Actors : OverlappingActors) {
		TotalMass += Actors->FindComponentByClass<UPrimitiveComponent>()->GetMass();
	}

	return TotalMass;
}
