// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralRoom.h"
#include "DrawDebugHelpers.h"
#include "FloorNode.h"
#include "Floor.h"
#include "Components/InstancedStaticMeshComponent.h"

#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "NavigationData.h"



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
	/* ------------------------------------------------ */
	PathMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PATHMESH"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PSM(TEXT("StaticMesh'/Game/Assets/Floor_400x400.Floor_400x400'"));
	if (PSM.Succeeded())
	{
		PathMesh->SetStaticMesh(PSM.Object);
	}

	PathMaterial = CreateDefaultSubobject<UMaterial>(TEXT("PATHMATERIAL"));
	static ConstructorHelpers::FObjectFinder<UMaterial> PM(TEXT("Material'/Game/Assets/M_PathMaterial.M_PathMaterial'"));
	if (PM.Succeeded())
	{
		PathMaterial = PM.Object;
	}

	PathMesh->SetVisibility(false);
	/* ------------------------------------------------ */
	PathWidth = 50.f;

	/*StartLocation = FVector(-400.f, -1000.f, 300.f);
	EndLocation = FVector(800.f, 1000.f, 300.f);*/
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
	TheFloor->DrawGrid(GetWorld());

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
	UE_LOG(LogTemp, Warning, TEXT("Partitioned Block Count : %d"), TheFloor->GetPartitionedFloor().Num());
	TheFloor->SpawnRoom(GetWorld());

	//TheFloor->PathFind(GetWorld());
	/* ------------------------------------------------------------------------------------------------ */
	// TODO : Move Sematnics
	/*Hallways = TheFloor->GetHallways();*/
	

	


	/*	계획
	*	그 다음, 강의 영상에서 알려준 복도 생성을 위한 thresholds, 문지방을 방내에서 선택해야
	*	하는데, 지금으로써 생각할 수 있는 방법은 방 내에서 배열과 같이 모든 그리드들의 좌표를 들고 있다가, 
		MST에서 Edge를 받아왔을 때 그 양끝점좌표로 부터 서로 가장 가까운 타일을 두개 선택해야한다는 것이다
		 그 두개의 타일 좌표를 AStar에 넘겨 
	*	길찾기를 수행해 복도를 만드는 방법이다.
	*	그럼 복도 타일들을 가지고있는 TArray와 양끝 문지방 타일 좌표번호를 알고 있으므로 실제 복도
	*	지형과 문도 생성이 가능할 것이다.
	*	그리고 영상 말미에 방들의 시작과 끝을 지정하고 싶을 경우, 각각의 정점과 Edge들에다가
	*	다시 BFS를 수행해 홉들을 카운트한다고 했는데, 가장 길이가 길게 나온 두 개의 정점을 
	*	던전의 시작과 끝 지점으로정한다. 단 이것은 던전의 설계에 따라 선택하여 추가한다
	*	계획 중 가장 최우선시 되어야할 사항은 역시 기존 설계가 잘못되어 실제 float 중점좌표로
	*	계산되었으므로 이를 Grid 기반으로 바꾸어야 한다는 것
		위에서 지금으로써 생각할 수 있는 방법이라고 한 부분에서,
	*	방의 꼭짓점을 제외한 모든 모서리들을 전부 가지고 두 방사이의 모서리들의 거리
	*	를 전부 계산 후 가장 가까운 2개의 모서리를 문지방으로 선택하여 길찾기를 수행하는 것이
	*	맞는 것같다.
	*  [ □□□ ]
	*  [ □□□ ] 
	*  [ □□□ ] 
	*  [ □□□ ]
	*  (x, y) 4 x 3의 방이 있는 경우, 각각 그리드의 좌표는 그리드 기반으로 어떻게 정의될까?
	*  좌측 최상단의 좌표는 당연 UpperLeftX, UpperLeftY,
	*  그럼 그 우측은?UpperLeftX, UpperLeftY + 1, +2....
	*  그럼 그 아래는 UpperLeftX+1, UpperLeftY
	*  => 배열의 index 처리와 같다
	*/
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

void AProceduralRoom::BuildPath(const FVector2D& StartLocation2D, const FVector2D& EndLocation2D)
{
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("NavSystem Not Found. !!"));
		return;
	}

	FPathFindingQuery Query;
	Query.StartLocation = FVector(StartLocation2D, 0.f);
	Query.EndLocation = FVector(EndLocation2D, 0.f);
	Query.NavData = NavSystem->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
	Query.Owner = GetOwner();

	FPathFindingResult Result = NavSystem->FindPathSync(Query);
	
	if (Result.IsSuccessful() && Result.Path.IsValid())
	{
		TArray<FNavPathPoint> PathPoints = Result.Path->GetPathPoints();

		UE_LOG(LogTemp, Warning, TEXT("%d"), PathPoints.Num());
		for (int32 i = 0; i < PathPoints.Num() - 1; i++)
		{
			DrawDebugPoint(GetWorld(), PathPoints[i].Location + FVector(0.f, 0.f, 1700.f), 30.f, FColor::Red, true, -1.f, 0);
			// TODO : DrawDebugPoint Last Index of PathPoints
			DrawDebugLine(GetWorld(), PathPoints[i].Location + FVector(0.f, 0.f, 1700.f), PathPoints[i + 1].Location + FVector(0.f, 0.f, 1700.f), FColor::Blue, true, -1.f, 0, 100.f);
			UE_LOG(LogTemp, Warning, TEXT("%f %f %f"), PathPoints[i].Location.X, PathPoints[i].Location.Y, PathPoints[i].Location.Z);
		}

		/*for (int32 i = 0; i < PathPoints.Num() - 1; i++)
		{
			FVector StartPoint = PathPoints[i].Location;
			FVector EndPoint = PathPoints[i + 1].Location;

			FVector PathSegment = StartPoint;
			float PathSegmentLength = PathSegment.X;
			FRotator PathSegmentRotation = PathSegment.Rotation();

			UStaticMeshComponent* PathMeshComponent = NewObject<UStaticMeshComponent>(this);
			PathMeshComponent->SetStaticMesh(PathMesh->GetStaticMesh());
			PathMeshComponent->SetMaterial(0, PathMaterial);
			PathMeshComponent->SetWorldLocation(StartPoint);
			PathMeshComponent->SetWorldRotation(PathSegmentRotation);


			PathMeshComponent->SetWorldScale3D(FVector(0.2f, 0.2f, 1.f));
			PathMeshComponent->RegisterComponent();
			PathMeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
		}*/
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Pathfinding Failed !!!!!"));
	}

}
