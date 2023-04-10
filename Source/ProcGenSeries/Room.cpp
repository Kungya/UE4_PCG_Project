// Fill out your copyright notice in the Description page of Project Settings.


#include "Room.h"
#include "Floor.h"
#include "FloorNode.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Templates/SharedPointer.h"

// Sets default values
ARoom::ARoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RoomFloor = CreateDefaultSubobject<UStaticMeshComponent>("RoomFloor");
	SetRootComponent(RoomFloor);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> RFS(TEXT("StaticMesh'/Game/Assets/SM_Template_Map_Floor.SM_Template_Map_Floor'"));
	static ConstructorHelpers::FObjectFinder<UMaterial> RFM(TEXT("Material'/Game/Assets/M_RoomFloor.M_RoomFloor'"));
	// Set Floor
	if (RFS.Succeeded() && RFM.Succeeded())
	{
		RoomFloor->SetStaticMesh(RFS.Object);
		RoomFloor->SetMaterial(0, RFM.Object);
		RoomFloor->SetCastShadow(false);
	}

	RoomRoof = CreateDefaultSubobject<UStaticMeshComponent>("RoomRoof");

	static ConstructorHelpers::FObjectFinder<UStaticMesh> RRS(TEXT("StaticMesh'/Game/Assets/SM_Template_Map_Floor.SM_Template_Map_Floor'"));
	static ConstructorHelpers::FObjectFinder<UMaterial> RRM(TEXT("Material'/Game/Assets/M_RoomFloor.M_RoomFloor'"));
	// Set Roof
	if (RRS.Succeeded() && RRM.Succeeded())
	{
		RoomRoof->SetStaticMesh(RRS.Object);
		RoomRoof->SetMaterial(0, RRM.Object);
		RoomRoof->SetCastShadow(false);
	}

	RoomWall = CreateDefaultSubobject<UStaticMeshComponent>("RoomWall");
	InstancedRoomWall = CreateDefaultSubobject<UInstancedStaticMeshComponent>("InstancedRoomWall");

	static ConstructorHelpers::FObjectFinder<UStaticMesh> RWS(TEXT("StaticMesh'/Game/Assets/Wall_400x400.Wall_400x400'"));	
	static ConstructorHelpers::FObjectFinder<UMaterial> RWM(TEXT("Material'/Game/Assets/M_RoomWall.M_RoomWall'"));
	// Set Wall
	if (RWS.Succeeded() && RWM.Succeeded())
	{
		InstancedRoomWall->SetStaticMesh(RWS.Object);
		InstancedRoomWall->SetMaterial(0, RWM.Object);
		InstancedRoomWall->SetCastShadow(false);
	}

	RoomWallThicknessFactor = 1.f;
	RoomHeightFactor = 4;
}

void ARoom::OnConstruction(const FTransform& Transform)
{ // Floor에서 SpawnActor로 Room이e 스폰될 때, Room의 Transform을 가지고 실행됨
	Super::OnConstruction(Transform);

	ConstructRooms(Transform);
}

// Called when the game starts or when spawned
void ARoom::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARoom::ConstructRooms(const FTransform& Transform)
{
	
	//UE_LOG(LogTemp, Warning, TEXT("Width : %f, Ratio : %f"), Node->GetRoomWidth() * UnitLength, Node->GetRoomWidth() * UnitLength / 400.f);

	RoomRoof->SetRelativeLocation(GetRootComponent()->GetRelativeLocation() + FVector(0.f, 0.f, RoomHeightFactor * 400.f));

	// Node.IsValid() 미체크시 런타임 이외에도 타 클래스 또는 블루프린트에서 Null 상태인 TSharedPtr 참조를 시도해 Crash 발생 가능성 유의
	if (InstancedRoomWall->GetInstanceCount() == 0 && Node.IsValid())
	{
		SetStaticMeshSize(FloorSize);

		Transforms.Push(FTransform(FRotator(0.f, 0.f, 0.f), FVector(Node->GetCornerCoordinates().RoomUpperLeftX * UnitLength, Node->GetCornerCoordinates().RoomUpperLeftY * UnitLength, 0.f), FVector(Node->GetRoomWidth() * UnitLength / 400.f, RoomWallThicknessFactor, RoomHeightFactor)));
		Transforms.Push(FTransform(FRotator(0.f, 90.f, 0.f), FVector(Node->GetCornerCoordinates().RoomLowerRightX * UnitLength, Node->GetCornerCoordinates().RoomUpperLeftY * UnitLength, 0.f), FVector(Node->GetRoomLength() * UnitLength / 400.f, RoomWallThicknessFactor, RoomHeightFactor)));
		Transforms.Push(FTransform(FRotator(0.f, 180.f, 0.f), FVector(Node->GetCornerCoordinates().RoomLowerRightX * UnitLength, Node->GetCornerCoordinates().RoomLowerRightY * UnitLength, 0.f), FVector(Node->GetRoomWidth() * UnitLength / 400.f, RoomWallThicknessFactor, RoomHeightFactor)));
		Transforms.Push(FTransform(FRotator(0.f, 270.f, 0.f), FVector(Node->GetCornerCoordinates().RoomUpperLeftX * UnitLength, Node->GetCornerCoordinates().RoomLowerRightY * UnitLength, 0.f), FVector(Node->GetRoomLength() * UnitLength / 400.f, RoomWallThicknessFactor, RoomHeightFactor)));
		
		//UE_LOG(LogTemp, Warning, TEXT("Transforms Num() : %d"), Transforms.Num());

		// loop 4(number of walls) times
		for (int32 i = 0; i < Transforms.Num(); i++)
		{
			InstancedRoomWall->AddInstance(Transforms[i]);
		}
	}
}

void ARoom::SetStaticMeshSize(const FVector& TargetUnitSize)
{
	FVector OriginalFloorSize = RoomFloor->GetStaticMesh()->GetBoundingBox().GetSize();
	
	FVector FloorRatio = FVector(TargetUnitSize.X / OriginalFloorSize.X, TargetUnitSize.Y / OriginalFloorSize.Y, TargetUnitSize.Z / OriginalFloorSize.Z);
	
	RoomFloor->SetRelativeScale3D(FloorRatio);
	RoomRoof->SetRelativeScale3D(FloorRatio);
}

void ARoom::SetUnitLength(float NewUnitLength)
{
	this->UnitLength = NewUnitLength;
}

// Called in Floor.cpp
void ARoom::SetRoomSize(const FVector& TargetSize)
{
	// Used in SetStaticMeshSize() to Scale Floor, Roof
	FloorSize = TargetSize * UnitLength;
}

void ARoom::SetNode(TSharedPtr<FloorNode> NewNode)
{
	Node = NewNode;
}