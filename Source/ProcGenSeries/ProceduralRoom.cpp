// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralRoom.h"
#include "DrawDebugHelpers.h"
#include "FloorNode.h"
#include "Floor.h"
#include "Components/InstancedStaticMeshComponent.h"

// Sets default values
AProceduralRoom::AProceduralRoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	SetRootComponent(FloorMesh);

	// 방의 크기는 1000x100, 5개로 분할시 하나당 길이는 200
	GridSizeX = 5;
	GridSizeY = 5;	
	SquareWidth = 200.f;

	TopLeft = FVector(0.f);
	BottomRight = FVector(1000.f, 1000.f, 0.f);
	GridHeight = 1.f;
	RoomLength = 1000.f;
	RoomWidth = 1000.f;

	Radius = 25.f;
}

// Called when the game starts or when spawned
void AProceduralRoom::BeginPlay()
{
	Super::BeginPlay();

	/*CreateGrid();
	PlacePointsOnGrid();*/

	TSharedPtr<Floor> TheFloor(new Floor());

	// 분할, 이 결과로 분할된 Floor들이 PartitionedFloor 배열에 저장 되어 있음
	TheFloor->Partition();
	// 분할된 부분 Grid로 보여줌
	TheFloor->DrawFloorNodes(GetWorld());

	// 일정 크기 이상 조건을 충족하는 FloorNode 들을 기존 길이의 80%로 줄여 따로 RoomCandidates에 저장시킴 
	TheFloor->SelectRoomCandiate(GetWorld());
	// RoomCandidates들의 선택 전 면적과 선택 후 면적을 Editor에 출력해줌
	TheFloor->DrawRoomNodes(GetWorld());

	// TODO : RoomCandidates 위치에 맞는 Room을 만들어 소환한다
	/*
	이 과정에서 현재 생각 해볼 수 있는 방법은 Room 이라는 Actor에 Instanced Static Mesh Component를 붙여
	방 형태로 만들고, 여기 class에서 SpawnActor를 이용해서 소환,
	고려해야할 점은 SpawnActor를 호출할 때마다 그 때 FloorNode 크기에 맞게 Room Actor 크기를 조정해놔야한다는 것
	
	시도해볼만한 방법들
	1) Room Actor에서 방 크기에 대한 변수를 만들어 두고, RoomCandidates[i]에 있는 좌표, 크기 인자를
	SetRoomSize라는 함수에 넘겨 그 때마다 RoomSize를 변경한 다음, 그 actor를 spawn하기
	*/

	TheFloor->SpawnRoom(GetWorld());
	

	UE_LOG(LogTemp, Warning, TEXT("Partitioned Block Count : %d"), TheFloor->GetPartitionedFloor().Num());

	//PickRoomCandidate(TheFloor);
}

// Called every frame
void AProceduralRoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProceduralRoom::SpawnItem(UClass* ItemToSpawn)
{
	float XCoordinate = FMath::FRandRange(0.f, 1000.f);
	float YCoordinate = FMath::FRandRange(0.f, 1000.f);

	float Yaw = FMath::FRandRange(0.f, 360.f);

	FVector Location(XCoordinate, YCoordinate, 0.f);
	FRotator Rotation(0.f, Yaw, 0.f);

	GetWorld()->SpawnActor<AActor>(ItemToSpawn, Location, Rotation);
}

void AProceduralRoom::CreateGrid()
{
	for (int32 i = 0; i < GridSizeX + 1; i++)
	{
		FVector Start = TopLeft + FVector(i * SquareWidth, 0.f, GridHeight);
		FVector End = Start + FVector(0.f, RoomLength, GridHeight);
		DrawDebugLine(GetWorld(), Start, End, FColor::Blue, true);
	}
	
	for (int32 i = 0; i < GridSizeX + 1; i++)
	{
		FVector Start = TopLeft + FVector(0.f, i * SquareWidth, GridHeight);
		FVector End = Start + FVector(RoomWidth, 0.f, GridHeight);
		DrawDebugLine(GetWorld(), Start, End, FColor::Blue, true);
	}
	
}

FVector AProceduralRoom::GetRandomPointInSquare(const FVector& UpperLeft, const FVector& LowerRight)
{
	float RandomX = FMath::FRandRange(UpperLeft.X, LowerRight.X);
	float RandomY = FMath::FRandRange(UpperLeft.Y, LowerRight.Y);

	return FVector(RandomX, RandomY, 0.f);
}

void AProceduralRoom::PlacePointsOnGrid()
{
	for (int32 i = 0; i < GridSizeX; i++)
	{
		for (int32 j = 0; j < GridSizeY; j++)
		{
			FVector UpperLeft(i * SquareWidth + Radius, j * SquareWidth + Radius, GridHeight);
			FVector LowerRight(i * SquareWidth + SquareWidth - Radius, j * SquareWidth + SquareWidth - Radius, GridHeight);
			FVector RandomPointInSquare = GetRandomPointInSquare(UpperLeft, LowerRight);
			
			DrawDebugPoint(GetWorld(), RandomPointInSquare, 5.f, FColor::Red, true);

			DrawDebugCircle(GetWorld(), RandomPointInSquare, Radius, 48, FColor::Orange, true, -1.f, 0, 2.5f, FVector(0.f, 1.f, 0.f), FVector(1.f, 0.f, 0.f), true);

			float RandomYaw = FMath::FRandRange(0.f, 360.f);
			GetWorld()->SpawnActor<AActor>(ChairClass, RandomPointInSquare, FRotator(0.f, RandomYaw, 0.f));
		}
	}
}