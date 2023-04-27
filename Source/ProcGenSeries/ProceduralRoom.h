// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralRoom.generated.h"

UCLASS()
class PROCGENSERIES_API AProceduralRoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProceduralRoom();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Room, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* FloorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Room, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> ChairClass;

	void SpawnItem(UClass* ItemToSpawn);

	float SquareWidth;
	float GridHeight;
	float RoomLength;
	float RoomWidth;

	float Radius;

	FVector TopLeft;
	FVector BottomRight;

	int32 GridSizeX;
	int32 GridSizeY;

	void CreateGrid();

	FVector GetRandomPointInSquare(const FVector& UpperLeft, const FVector& LowerRight);

	void PlacePointsOnGrid();

public:
	//void PickRoomCandidate(class TSharedPtr<Floor> CandidateFloor);

	// Segment to contruct Room Wall Mesh 
private:
	class UInstancedMeshComponent* InstancedRoomMesh;

	/* Pathfinding Segment */
private:
	UPROPERTY(VisibleAnywhere, Category = "Pathfinding")
	float PathWidth;

	UPROPERTY(VisibleAnywhere, Category = "Pathfinding")
	UStaticMeshComponent* PathMesh;

	UPROPERTY(VisibleAnywhere, Category = "Pathfinding")
	UMaterialInterface* PathMaterial;

	TArray<TPair<FVector2D, FVector2D>> Hallways;
public:
	void BuildPath(const FVector2D& StartLocation2D, const FVector2D& EndLocation2D);

};
