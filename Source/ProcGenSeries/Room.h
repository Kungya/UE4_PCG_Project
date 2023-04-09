// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FloorNode.h"
#include "GameFramework/Actor.h"
#include "Room.generated.h"

UCLASS()
class PROCGENSERIES_API ARoom : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARoom();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ConstructRooms(const FTransform& Transform);

	void SetStaticMeshSize(const FVector& TargetUnitSize);

	void SetUnitLength(float NewUnitLength);

	void SetRoomSize(const FVector& TargetSize);

	void SetNode(TSharedPtr<FloorNode> NewNode);
	
	/*FORCEINLINE void SetCornerCoordinates(FCornerCoordinates Coordinates) { CornerCoordinates = Coordinates; }
	FORCEINLINE FCornerCoordinates GetCornerCoordinates() const { return CornerCoordinates; }*/
private:

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* RoomFloor;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RoomRoof;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RoomWall;

	UPROPERTY(VisibleAnywhere)
	class UInstancedStaticMeshComponent* InstancedRoomWall;

	UPROPERTY(Transient)
	TArray<FTransform> Transforms;

	FVector FloorSize;

	float UnitLength;

	float RoomWallThicknessFactor;
	float RoomHeightFactor;

	TSharedPtr<FloorNode> Node;

	/*FCornerCoordinates CornerCoordinates;*/
};