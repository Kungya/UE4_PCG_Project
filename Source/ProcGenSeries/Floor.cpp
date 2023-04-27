#include "Floor.h"
#include "FloorNode.h"
#include "Room.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

Floor::Floor()
{
	FloorGridSizeX = 50;
	FloorGridSizeY = 50;
	RoomMinX = 4;
	RoomMinY = 4;

	RoomMinArea = 40 ;
	RoomMinRatio = 0.25f;
	RoomMinWidth = 4;
	RoomMinLength = 4;

	// 실제 길이와 면적은 UnitLength에 곱하여 결정 됨
	// 그러므로 GetArea등에서 이용되는 면적은 UnitLength에 곱해지기 전의 값.
	UnitLength = 500;

	SplitChance = 1.75f;

	ReincorporationChance = 12.5f;

	// Initialize 1 ("1" means block that can move)
	for (int32 y = 0; y < FloorGridSizeY; y++)
	{
		for (int32 x = 0; x < FloorGridSizeX; x++)
		{
			PathFindGrid[y][x] = 1;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Floor Created"));
}

Floor::~Floor()
{
	UE_LOG(LogTemp, Warning, TEXT("Floor Destroyed"));
}

void Floor::Partition()
{ // This function is executed at beginplay() in AProceduralRoom class
	
	FCornerCoordinates CornerCoordinatesA = { 0, 0, FloorGridSizeX, FloorGridSizeY };

	FloorNodeStack.Push(TSharedPtr<FloorNode>(new FloorNode(CornerCoordinatesA)));

	while (FloorNodeStack.Num() > 0)
	{
		TSharedPtr<FloorNode> A = FloorNodeStack.Pop();

		// 동전을 던져서 나오는 수평/수직 방향 중 하나로 먼저 분할을 시도
		bool bNodeWasSplit = SplitAttempt(A);
		if (!bNodeWasSplit)
		{ // 분할되지 않았으면 이미 분할이 다 되어 최소 블럭 상태이므로 최종적으로 PartitionedFloor에 Push
			PartitionedFloor.Push(A);
		}
	}

}

int32 Floor::CoinFlip()
{
	return FMath::RandRange(0, 1);
}

bool Floor::ShouldSplitNode(TSharedPtr<FloorNode> InNode, ESplitOrientation Orientation)
{ // 현재 Floor의 Length/Width가 최소(RoomMinY or X)보다 큰가?
	FCornerCoordinates CornerCoordinates = InNode->GetCornerCoordinates();

	if (Orientation == ESplitOrientation::ESO_Horizontal)
	{
		int32 FloorLength = CornerCoordinates.LowerRightY - CornerCoordinates.UpperLeftY;
		// 0~1
		float PercentChanceOfSplit = (float)FloorLength / (float)FloorGridSizeY;
		PercentChanceOfSplit *= SplitChance;

		float DiceRoll = FMath::FRandRange(0.f, 1.f);

		// 많이 분할되어 길이가 작은 Floor 일수록 Percent가 낮아져 분할될 확률 감소.
		if (DiceRoll > PercentChanceOfSplit)
		{ // don't split by the DiceRoll
			return false;
		}

		// new Floor's Length after Split is longer than (including equal) RoomMinY, so (2 * RoomMinY)
		if (FloorLength >= 2 * RoomMinY)
		{ // can split
			return true;
		}
	}
	else if (Orientation == ESplitOrientation::ESO_Vertical)
	{ 
		int32 FloorWidth = CornerCoordinates.LowerRightX - CornerCoordinates.UpperLeftX;
		// 0~1
		float PercentChanceOfSplit = (float)FloorWidth / (float)FloorGridSizeX;
		PercentChanceOfSplit *= SplitChance;
		
		float DiceRoll = FMath::FRandRange(0.f, 1.f);

		// 많이 분할되어 길이가 작은 Floor 일수록 Percent가 낮아져 분할될 확률 감소.
		if (DiceRoll > PercentChanceOfSplit)
		{ // don't split by the DiceRoll
			return false;
		}

		// new Floor's Length after Split is longer than (including equal) RoomMinX, so (2 * RoomMinX)
		if (FloorWidth >= 2 * RoomMinX)
		{ // can split
			return true;
		}
	}

	// can't split, end partition
	return false;
}

bool Floor::SplitAttempt(TSharedPtr<FloorNode> InNode)
{
	int32 HorizontalOrVertical = CoinFlip();

	if (HorizontalOrVertical == 0)
	{
		// split 가능한 길이를 갖고 있는가?
		if (ShouldSplitNode(InNode, ESplitOrientation::ESO_Horizontal))
		{
			// Try to split Horizontally "first"
			TSharedPtr<FloorNode> B(new FloorNode());
			TSharedPtr<FloorNode> C(new FloorNode());

			SplitHorizontal(InNode, B, C);
			return true;
		}
		else if (ShouldSplitNode(InNode, ESplitOrientation::ESO_Vertical))
		{
			// Try to split Vertically
			TSharedPtr<FloorNode> B(new FloorNode());
			TSharedPtr<FloorNode> C(new FloorNode());

			SplitVertical(InNode, B, C);
			return true;
		}
	}
	else if (HorizontalOrVertical == 1)
	{
		if (ShouldSplitNode(InNode, ESplitOrientation::ESO_Vertical))
		{
			// Try to split Vertically "first"
			TSharedPtr<FloorNode> B(new FloorNode());
			TSharedPtr<FloorNode> C(new FloorNode());

			SplitVertical(InNode, B, C);
			return true;

		}
		else if (ShouldSplitNode(InNode, ESplitOrientation::ESO_Horizontal))
		{
			// Try to split Horizontally
			TSharedPtr<FloorNode> B(new FloorNode());
			TSharedPtr<FloorNode> C(new FloorNode());

			SplitHorizontal(InNode, B, C);
			return true;
		}
	}

	// Can't Split, maybe end of partition
	return false;
}

void Floor::SplitHorizontal(TSharedPtr<FloorNode> InA, TSharedPtr<FloorNode> InB, TSharedPtr<FloorNode> InC)
{
	/*
	끝 부분을 분할 하는 것은 말이 안되므로 RoomMinY를 추가하여 예외 방지
	추후 분할되는 방의 최소 크기 조정을 위해 RoomMinY 변경 고려 가능
	*/
	int32 SplitPointY = FMath::RandRange(InA->GetCornerCoordinates().UpperLeftY + RoomMinY, InA->GetCornerCoordinates().LowerRightY - RoomMinY);

	// A를 수평 분할하여 새로운 B, C 설정

	// The Only Diff of B is LowerRIghtY.
	FCornerCoordinates CornerCoordinatesB;
	CornerCoordinatesB.UpperLeftX = InA->GetCornerCoordinates().UpperLeftX;
	CornerCoordinatesB.UpperLeftY = InA->GetCornerCoordinates().UpperLeftY;
	CornerCoordinatesB.LowerRightX = InA->GetCornerCoordinates().LowerRightX;
	CornerCoordinatesB.LowerRightY = SplitPointY;

	InB->SetCornerCoordinates(CornerCoordinatesB);
	FloorNodeStack.Push(InB);

	// The Only Diff of C is UpperLeftY.
	FCornerCoordinates CornerCoordinatesC;
	CornerCoordinatesC.UpperLeftX = InA->GetCornerCoordinates().UpperLeftX;
	CornerCoordinatesC.UpperLeftY = SplitPointY;
	CornerCoordinatesC.LowerRightX = InA->GetCornerCoordinates().LowerRightX;
	CornerCoordinatesC.LowerRightY = InA->GetCornerCoordinates().LowerRightY;

	InC->SetCornerCoordinates(CornerCoordinatesC);
	FloorNodeStack.Push(InC);
}

void Floor::SplitVertical(TSharedPtr<FloorNode> InA, TSharedPtr<FloorNode> InB, TSharedPtr<FloorNode> InC)
{
	/*
	끝 부분을 분할 하는 것은 말이 안되므로 RoomMinX를 추가하여 예외 방지
	추후 분할되는 방의 최소 크기 조정을 위해 RoomMinX 변경 고려 가능
	*/
	int32 SplitPointX = FMath::RandRange(InA->GetCornerCoordinates().UpperLeftX + RoomMinX, InA->GetCornerCoordinates().LowerRightX - RoomMinX);

	// A를 수직 분할하여 새로운 B, C 설정

	// The Only Diff of B is LowerRightX.
	FCornerCoordinates CornerCoordinatesB;
	CornerCoordinatesB.UpperLeftX = InA->GetCornerCoordinates().UpperLeftX;
	CornerCoordinatesB.UpperLeftY = InA->GetCornerCoordinates().UpperLeftY;
	CornerCoordinatesB.LowerRightX = SplitPointX;
	CornerCoordinatesB.LowerRightY = InA->GetCornerCoordinates().LowerRightY;

	InB->SetCornerCoordinates(CornerCoordinatesB);
	FloorNodeStack.Push(InB);

	// The Only Diff of C is UpperLeftX;
	FCornerCoordinates CornerCoordinatesC;
	CornerCoordinatesC.UpperLeftX = SplitPointX;
	CornerCoordinatesC.UpperLeftY = InA->GetCornerCoordinates().UpperLeftY;
	CornerCoordinatesC.LowerRightX = InA->GetCornerCoordinates().LowerRightX;
	CornerCoordinatesC.LowerRightY = InA->GetCornerCoordinates().LowerRightY;
	
	InC->SetCornerCoordinates(CornerCoordinatesC);
	FloorNodeStack.Push(InC);
}

void Floor::DrawFloorNodes(UWorld* World)
{
	for (int32 i = 0; i < PartitionedFloor.Num(); i++)
	{
		FCornerCoordinates Coordinates = PartitionedFloor[i]->GetCornerCoordinates();
		DrawFloorNode(World, Coordinates, 20.f, FColor::Yellow);
	}
}

void Floor::DrawFloorNode(UWorld* World, FCornerCoordinates Coordinates, int32 count, FColor NodeColor)
{
	// count * 10 in Z axis : to distinguish by height
	const FVector UpperLeft(Coordinates.UpperLeftX * UnitLength, Coordinates.UpperLeftY * UnitLength, count * 10);
	const FVector UpperRight(Coordinates.LowerRightX * UnitLength, Coordinates.UpperLeftY * UnitLength, count * 10);
	const FVector LowerLeft(Coordinates.UpperLeftX * UnitLength, Coordinates.LowerRightY * UnitLength, count * 10);
	const FVector LowerRight(Coordinates.LowerRightX * UnitLength, Coordinates.LowerRightY * UnitLength, count * 10);

	
	DrawDebugLine(World, UpperLeft, UpperRight, NodeColor, true, -1, 0, 50.f);
	DrawDebugLine(World, UpperLeft, LowerLeft, NodeColor, true, -1, 0, 50.f);
	DrawDebugLine(World, LowerRight, UpperRight, NodeColor, true, -1, 0, 50.f);
	DrawDebugLine(World, LowerRight, LowerLeft, NodeColor, true, -1, 0, 50.f);
}

void Floor::DrawGrid(UWorld* World)
{
	// (0, 0) ~ (FloorGridSizeX * UnitLength, FloorGridSizeY * UnitLength),  여기선 (25000, 25000)
	for (int32 x = 0; x < FloorGridSizeX; x++)
	{
		for (int32 y = 0; y < FloorGridSizeY; y++)
		{
			FCornerCoordinates Grid;
			Grid.UpperLeftX = x;
			Grid.UpperLeftY = y;
			Grid.LowerRightX = (x + 1);
			Grid.LowerRightY = (y + 1);
			DrawFloorNode(World, Grid, 10.f, FColor::Black);
		}
	}
}

void Floor::SelectRoomCandiate(UWorld* World)
{
	// 1) 일정 조건(크기 80 이상)을 걸어 조건을 만족하는 Floor들을 Patitioned Floor 배열에서 골라낸다
	// 2) 골라낸 새로운 배열에서 골라서 방을 생성하기 전, 크기를 그대로 설정할 경우 인접한 방과의 겹침이
	// 발생할 수 있으니 크기를 1씩 줄인다. -> 1:9, 9:1 내분점을 만들어 각각의 길이를 20%줄여 이전 면적의 64%로 축소하기로 함
	// ** 이 과정에서 방 크기가 사라지거나 심하게 작아질 수 있으므로 이전 단계에서 
	// 너무 작은 방들은 전부 제외하고 중하위 크기 이상의 방들만 골라내는 것 -> 일정 크기 이상의 방이어야 플레이 가능함
	// 기존의 80%로 만든다, 그럼 상하좌우 각각 10%씩 감소시키면 되므로
	// 중점을 구하는 방법과 비슷하게 양끝단 좌표를 더하고 0.1 곱하면 좌측상단 기존의 90%에 해당하는 좌표 생성..
	
	/*	길찾기를 위해 위의 방법이 아닌 격자 방식으로 변경한다. 격자는 이미 정의되어 있으므로 기존에 90%에 해당하는 길이로 
	*	만드는 것이 아니라 분할된 FloorNode의 좌표에서 각각 1 UnitLength 만큼의 여유분을 두고 축소 시킨다.
	*	이렇게 하면 테두리 끝점을 제외한 내부 방들의 사이는 2 UnitLength (= 10m) 만큼의 여유를 격자 단위로 갖게되며,
	*	추후 복도가 맵밖으로 돌아가야할 수도 있는 최악의 경우를 제외하고 격자를 넣은 int 배열에서 길찾기가 가능해진다.
	*  -> 원활한 복도 생성을위해 방의 가로, 세로 길이가 최소 5 Unitlength가 되도록한다.4일 경우 2가 감소되어 2칸밖에 존재하지않아 방의 길이가 너무 좁아지게 된다.
	*/
	for (int32 i = 0; i < PartitionedFloor.Num(); i++)
	{
		if (PartitionedFloor[i]->GetArea() > RoomMinArea && 
			PartitionedFloor[i]->GetFloorNodeRatio() > RoomMinRatio && 
			PartitionedFloor[i]->GetWidth() > RoomMinWidth && 
			PartitionedFloor[i]->GetLength() > RoomMinLength)
		{
			FCornerCoordinates Coordinates = PartitionedFloor[i]->GetCornerCoordinates();

			// 각각 1 UnitLength 만큼 뺀다
			Coordinates.RoomUpperLeftX = Coordinates.UpperLeftX + 1;
			Coordinates.RoomUpperLeftY = Coordinates.UpperLeftY + 1;
			Coordinates.RoomLowerRightX = Coordinates.LowerRightX - 1;
			Coordinates.RoomLowerRightY = Coordinates.LowerRightY - 1;

			/*Coordinates.RoomUpperLeftX = (Coordinates.UpperLeftX * 9 + Coordinates.LowerRightX) / 10.f;
			Coordinates.RoomUpperLeftY = (Coordinates.UpperLeftY * 9 + Coordinates.LowerRightY) / 10.f;
			Coordinates.RoomLowerRightX = (Coordinates.UpperLeftX + Coordinates.LowerRightX * 9) / 10.f;
			Coordinates.RoomLowerRightY = (Coordinates.UpperLeftY + Coordinates.LowerRightY * 9) / 10.f;*/

			PartitionedFloor[i]->SetCornerCoordinates(Coordinates);

			RoomCandidates.Push(PartitionedFloor[i]);
		}
	}
}

void Floor::DrawRoomNodes(UWorld* World)
{
	//생성된 FloorNode들의 중점 위치에 각각의 방 크기 표시
	for (int32 i = 0; i < RoomCandidates.Num(); i++)
	{
		//FVector MidPoint(RoomCandidates[i]->GetMidPointX() * UnitLength, RoomCandidates[i]->GetMidPointY() * UnitLength, 10.f);
		// Draw Area of Node and Room
		//DrawDebugString(World, MidPoint, FString::FromInt(RoomCandidates[i]->GetArea()), 0, FColor::Orange, -1.f, false, 3.f);
		//DrawDebugString(World, MidPoint + FVector(500.f, 500.f, 0.f), FString::SanitizeFloat(RoomCandidates[i]->GetRoomArea()), 0, FColor::Orange, -1.f, false, 3.f);
	}
}

void Floor::SpawnRoom(UWorld* World)
{
	for (int32 i = 0; i < RoomCandidates.Num(); i++)
	{
		FVector SpawnLocation(RoomCandidates[i]->GetMidPointX() * UnitLength, RoomCandidates[i]->GetMidPointY() * UnitLength, 0.f);
		
		// TODO : NewObject 분석 후 복습
		Room = ARoom::StaticClass();
		ARoom* NewRoom = NewObject<ARoom>(World, Room);
		// *★* Room이 spawn되고 OnConstruction내 에서 Insatnced Static Mesh 생성위해 OnConstruction 호출 연기.
		ARoom* RoomActor = World->SpawnActorDeferred<ARoom>(Room, FTransform(SpawnLocation));
		/* ------------------------------------------------------------------------------- */
		RoomActor->SetUnitLength(UnitLength);
		RoomActor->SetRoomSize(RoomCandidates[i]->GetRoomSize());
		RoomActor->SetNode(RoomCandidates[i]);
		/* ------------------------------------------------------------------------------- */
		UGameplayStatics::FinishSpawningActor(RoomActor, FTransform(SpawnLocation));

		// for Delaunay Triangulation
		RoomPointsArr.push_back(delaunay::Point<float>(RoomCandidates[i]->GetMidPointX() * UnitLength, RoomCandidates[i]->GetMidPointY() * UnitLength));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Count Room : %d"), RoomCandidates.Num());

	// Triangulate Rooms and Create MST
	Triangulation(World);
	// prim 알고리즘 진행
	CreateMST(World);
	// Cycle을 만들기 위해 확률적으로 Triangulation에 있던 간선 추가
	ReincorporateEdge(World);
	// 복도의 양끝 입구 결정
	SelectThreshold(World);
	// PathFind using A*
}

void Floor::Triangulation(UWorld* World)
{	
	// TODO : Convert TSharedPtr or not.
	delaunay::Delaunay<float> Triangulated = delaunay::triangulate(RoomPointsArr);

	for (auto& i : Triangulated.triangles)
	{
		DrawDebugLine(World, FVector(i.p0.x, i.p0.y, 1700.f), FVector(i.p1.x, i.p1.y, 1700.f), FColor::Green, true, -1, 0, 50.f);
		DrawDebugLine(World, FVector(i.p1.x, i.p1.y, 1700.f), FVector(i.p2.x, i.p2.y, 1700.f), FColor::Green, true, -1, 0, 50.f);
		DrawDebugLine(World, FVector(i.p2.x, i.p2.y, 1700.f), FVector(i.p0.x, i.p0.y, 1700.f), FColor::Green, true, -1, 0, 50.f);
	}

	for (auto& i : Triangulated.edges)
	{
		TriangulatedUniqueEdgesArr.AddUnique(i);
	}
	// 이제, TArray<delaunay::Edge<float>> TrinagulatedEdgeArr에 Edge들이 "중복되지 않게" 전부 들어있음.
	// Edge : Point 2개 소유, Point는 x, y좌표 각각 소유
}

void Floor::SetNodes(UWorld* World)
{
	for (int32 i = 0; i < RoomCandidates.Num(); i++)
	{
		Nodes.Emplace(Node(i, RoomCandidates[i]->GetMidPointX() * UnitLength, RoomCandidates[i]->GetMidPointY() * UnitLength));
		// 각 방의 위치에 좌표로 변환한 id 표시.
		//DrawDebugString(World, FVector(RoomCandidates[i]->GetMidPointX() * UnitLength, RoomCandidates[i]->GetMidPointY() * UnitLength, 500.f), FString::FromInt(i), 0, FColor::Orange, -1.f, false, 3.f);
	}
}

void Floor::CreateMST(UWorld* World)
{
	SetNodes(World);
	TUniquePtr<MinimumSpanningTree> MST(new MinimumSpanningTree(Nodes, TriangulatedUniqueEdgesArr, World));
	MinCostSum = MST->prim();
	UE_LOG(LogTemp, Warning, TEXT("Prim Result(최소가중치합) : %f"), MinCostSum);
	SetMSTEdges(MST->GetMSTEdges());
	/*for (auto& i : MSTEdges)
	{
		DrawDebugLine(World, FVector(i.Key.X, i.Key.Y, 1700.f), FVector(i.Value.X, i.Value.Y, 1700.f), FColor::Red, true, -1, 0, 100.f);
	}*/
}

void Floor::ReincorporateEdge(UWorld* World)
{
	Hallways = MoveTemp(MSTEdges);

	// TODO : 추후 프로토타입이 완성되고 복도 결합 기능까지 완성되었을 때 cycle 추가 예정
	// -> 현재 바로 cycle 추가시 복도의 입구가 겹치는 경우가 간혹 있음
	/*for (const auto& i : TriangulatedUniqueEdgesArr)
	{
		// 15%
		if (FMath::FRandRange(0.f, 100.f) < ReincorporationChance)
		{
			TPair<FVector2D, FVector2D> AddedEdge(FVector2D(i.p0.x, i.p0.y), FVector2D(i.p1.x, i.p1.y));
			if (AddUniqueEdge(Hallways, AddedEdge))
			{ // succeed AddUniqueEdge

			}
		}
	}*/

	UE_LOG(LogTemp, Warning, TEXT("%d"), Hallways.Num());

	for (const auto& i : Hallways)
	{
		DrawDebugLine(World, FVector(i.Key.X, i.Key.Y, 1700.f), FVector(i.Value.X, i.Value.Y, 1700.f), FColor::Red, true, -1, 0, 100.f);
	}
}

void Floor::SelectThreshold(UWorld* World)
{
	// first, Get CornerCoodinates of 2 Rooms from Edge in TArray<> Hallways
	for (const auto& i : Hallways)
	{ // select one Edge
		FCornerCoordinates RoomACoord;
		FCornerCoordinates RoomBCoord;

		for (const auto& j : RoomCandidates)
		{
			RoomACoord = j->TryGetCornerCoordinatesByMidPoint(i.Key.X / UnitLength, i.Key.Y / UnitLength);
			if (RoomACoord.UpperLeftX != -1)
			{ // succeed in Trying
				break;
			}
		}

		for (const auto& j : RoomCandidates)
		{
			RoomBCoord = j->TryGetCornerCoordinatesByMidPoint(i.Value.X / UnitLength, i.Value.Y / UnitLength);
			if (RoomBCoord.UpperLeftX != -1)
			{ // succeed in Trying
				break;
			}
		}

		/* Edge에 따른 각각 방의 양 끝점 좌표를 알게되었으니, 이제 방마다 양끝점 4개끼리 거리를 구한 뒤,
		* 가장 가까운 점 2개를 선택히고 그 점이 속한 모서리의 타일들 중에서 가장 가까운 타일 2개를 Threshold로
		* 지정한다 
		* -> 모든 모서리를 가지고 계산하는 것보다 시간 1/4 소모
		* 최악의 경우 : (25 * 4)^2, 10000번 계산, (12.5 * 4)^2-> 2500번 계산
		*/

		// RoomACoord, RoomBCoord

		TArray<FVector2D> RoomA;
		RoomA.Push(FVector2D(RoomACoord.RoomUpperLeftX * UnitLength, RoomACoord.RoomUpperLeftY * UnitLength));
		RoomA.Push(FVector2D(RoomACoord.RoomUpperLeftX * UnitLength, RoomACoord.RoomLowerRightY * UnitLength));
		RoomA.Push(FVector2D(RoomACoord.RoomLowerRightX * UnitLength, RoomACoord.RoomLowerRightY * UnitLength));
		RoomA.Push(FVector2D(RoomACoord.RoomLowerRightX * UnitLength, RoomACoord.RoomUpperLeftY * UnitLength));

		TArray<FVector2D> RoomB;
		RoomB.Push(FVector2D(RoomBCoord.RoomUpperLeftX * UnitLength, RoomBCoord.RoomUpperLeftY * UnitLength));
		RoomB.Push(FVector2D(RoomBCoord.RoomUpperLeftX * UnitLength, RoomBCoord.RoomLowerRightY * UnitLength));
		RoomB.Push(FVector2D(RoomBCoord.RoomLowerRightX * UnitLength, RoomBCoord.RoomLowerRightY * UnitLength));
		RoomB.Push(FVector2D(RoomBCoord.RoomLowerRightX * UnitLength, RoomBCoord.RoomUpperLeftY * UnitLength));

		FVector2D ClosestCornerA;
		FVector2D ClosestCornerB;

		float MinDist = TNumericLimits<float>::Max();
		float CurrentDist;

		// Euclidean Distance
		for (const FVector2D& A : RoomA)
		{
			for (const FVector2D& B : RoomB)
			{
				CurrentDist = FVector2D::DistSquared(A, B);

				if (CurrentDist < MinDist)
				{
					MinDist = CurrentDist;
					ClosestCornerA = A;
					ClosestCornerB = B;
				}
			}
		}

		// 가장 가까운 두 방의 모서리를 각각 구했으니 이제 이 모서리가 어느 쪽의 모서리인지 판별해야한다

		TArray<FVector2D> RoomATiles;
		int32 RoomAWidth = RoomACoord.RoomLowerRightX - RoomACoord.RoomUpperLeftX;
		int32 RoomALength = RoomACoord.RoomLowerRightY - RoomACoord.RoomUpperLeftY;

		float tolerance = 0.01f;
		// Room A
		if (FMath::IsNearlyEqual(ClosestCornerA.X, RoomACoord.RoomUpperLeftX * UnitLength, tolerance) && FMath::IsNearlyEqual(ClosestCornerA.Y, RoomACoord.RoomUpperLeftY * UnitLength, tolerance))
		{
			// ClosestCornerA는 좌측상단, 선택된 Room의 길이는 최소 3이 보장됨
			for (int32 X = 1; X <= RoomAWidth - 2; X++)
			{
				RoomATiles.Push(ClosestCornerA + FVector2D(X * UnitLength, 0));
			}

			for (int32 Y = 1; Y <= RoomALength - 2; Y++)
			{
				RoomATiles.Push(ClosestCornerA + FVector2D(0, Y * UnitLength));
			}
		}
		else if (FMath::IsNearlyEqual(ClosestCornerA.X, RoomACoord.RoomUpperLeftX * UnitLength, tolerance) && FMath::IsNearlyEqual(ClosestCornerA.Y, RoomACoord.RoomLowerRightY * UnitLength, tolerance))
		{
			// ClosestCornerA는 좌측하단, 선택된 Room의 길이는 최소 3이 보장됨
			for (int32 X = 1; X <= RoomAWidth - 2; X++)
			{
				RoomATiles.Push(ClosestCornerA + FVector2D(X * UnitLength, 0));
			}

			for (int32 Y = 1; Y <= RoomALength - 2; Y++)
			{
				RoomATiles.Push(ClosestCornerA + FVector2D(0, -Y * UnitLength));
			}
		}
		else if (FMath::IsNearlyEqual(ClosestCornerA.X, RoomACoord.RoomLowerRightX * UnitLength, tolerance) && FMath::IsNearlyEqual(ClosestCornerA.Y, RoomACoord.RoomLowerRightY * UnitLength, tolerance))
		{
			// ClosestCornerA는 우측하단, 선택된 Room의 길이는 최소 3이 보장됨
			for (int32 X = 1; X <= RoomAWidth - 2; X++)
			{
				RoomATiles.Push(ClosestCornerA + FVector2D(-X * UnitLength, 0));
			}

			for (int32 Y = 1; Y <= RoomALength - 2; Y++)
			{
				RoomATiles.Push(ClosestCornerA + FVector2D(0, -Y * UnitLength));
			}
		}
		else if (FMath::IsNearlyEqual(ClosestCornerA.X, RoomACoord.RoomLowerRightX * UnitLength, tolerance) && FMath::IsNearlyEqual(ClosestCornerA.Y, RoomACoord.RoomUpperLeftY * UnitLength, tolerance))
		{
			// ClosestCornerA는 우측상단, 선택된 Room의 길이는 최소 3이 보장됨
			for (int32 X = 1; X <= RoomAWidth - 2; X++)
			{
				RoomATiles.Push(ClosestCornerA + FVector2D(-X * UnitLength, 0));
			}

			for (int32 Y = 1; Y <= RoomALength - 2; Y++)
			{
				RoomATiles.Push(ClosestCornerA + FVector2D(0, Y * UnitLength));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Room A : 어느 쪽 모서리인지 판별 실패, 디버깅 필요"));
		}

		TArray<FVector2D> RoomBTiles;
		int32 RoomBWidth = RoomBCoord.RoomLowerRightX - RoomBCoord.RoomUpperLeftX;
		int32 RoomBLength = RoomBCoord.RoomLowerRightY - RoomBCoord.RoomUpperLeftY;

		// Room B
		if (FMath::IsNearlyEqual(ClosestCornerB.X, RoomBCoord.RoomUpperLeftX * UnitLength, tolerance) && FMath::IsNearlyEqual(ClosestCornerB.Y, RoomBCoord.RoomUpperLeftY * UnitLength, tolerance))
		{
			// ClosestCornerB는 좌측상단, 선택된 Room의 길이는 최소 3이 보장됨
			for (int32 Y = 1; Y <= RoomBLength - 2; Y++)
			{
				RoomBTiles.Push(ClosestCornerB + FVector2D(0, Y * UnitLength));
			}
			
			for (int32 X = 1; X <= RoomBWidth - 2; X++)
			{
				RoomBTiles.Push(ClosestCornerB + FVector2D(X * UnitLength, 0));
			}
		}
		else if (FMath::IsNearlyEqual(ClosestCornerB.X, RoomBCoord.RoomUpperLeftX * UnitLength, tolerance) && FMath::IsNearlyEqual(ClosestCornerB.Y, RoomBCoord.RoomLowerRightY * UnitLength, tolerance))
		{
			// ClosestCornerB는 좌측하단, 선택된 Room의 길이는 최소 3이 보장됨
			for (int32 Y = 1; Y <= RoomBLength - 2; Y++)
			{
				RoomBTiles.Push(ClosestCornerB + FVector2D(0, -Y * UnitLength));
			}
			
			for (int32 X = 1; X <= RoomBWidth - 2; X++)
			{
				RoomBTiles.Push(ClosestCornerB + FVector2D(X * UnitLength, 0));
			}
		}
		else if (FMath::IsNearlyEqual(ClosestCornerB.X, RoomBCoord.RoomLowerRightX * UnitLength, tolerance) && FMath::IsNearlyEqual(ClosestCornerB.Y, RoomBCoord.RoomLowerRightY * UnitLength, tolerance))
		{
			// ClosestCornerB는 우측하단, 선택된 Room의 길이는 최소 3이 보장됨
			for (int32 Y = 1; Y <= RoomBLength - 2; Y++)
			{
				RoomBTiles.Push(ClosestCornerB + FVector2D(0, -Y * UnitLength));
			}
			
			for (int32 X = 1; X <= RoomBWidth - 2; X++)
			{
				RoomBTiles.Push(ClosestCornerB + FVector2D(-X * UnitLength, 0));
			}
		}
		else if (FMath::IsNearlyEqual(ClosestCornerB.X, RoomBCoord.RoomLowerRightX * UnitLength, tolerance) && FMath::IsNearlyEqual(ClosestCornerB.Y, RoomBCoord.RoomUpperLeftY * UnitLength, tolerance))
		{
			// ClosestCornerB는 우측상단, 선택된 Room의 길이는 최소 3이 보장됨
			for (int32 Y = 1; Y <= RoomBLength - 2; Y++)
			{
				RoomBTiles.Push(ClosestCornerB + FVector2D(0, Y * UnitLength));
			}
			
			for (int32 X = 1; X <= RoomBWidth - 2; X++)
			{
				RoomBTiles.Push(ClosestCornerB + FVector2D(-X * UnitLength, 0));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Room B : 어느 쪽 모서리인지 판별 실패, 디버깅 필요"));
		}

		/* 이제, 방 A, B의 길찾기를 진행할 후보 타일이 4개, 4개씩 들어있다.
		*  여기서 가장 가까운 타일을 또 구해야하니 RommATiles와 RoomBTiles 를 반복문을 돌려서 가장 유클리드 거리가 작은 타일 2개를 구하고,
		* 그 타일의 좌표를 "정수화"해서 최종적으로 AStar 알고리즘을 진행하기 위한 준비를 마친다.
		*  위 코드가 엄청나게 길어 가독성이 떨어지지만 만약 방 A, B의 모든 타일을 후보로 두고 거리를 계산하면 횟수가 최대 1만번까지 많아진다... 만약 시간적 이점이 없다면 추후 리팩토링 해야함
		* -> 실제 구동 결과 시간적 이점보다 이렇게 되면 타일의 출발지가 너무 적어 타일이 겹치게 되는 경우가 생김, 모든 모서리 타일을 다 넣고 돌려야할듯 
		*/

		FVector2D ClosestTileA;
		FVector2D ClosestTileB;

		int32 MinTileDist = TNumericLimits<int32>::Max();
		int32 CurrentTileDist;
		
		// in32 변경이유 : float 오차로 X->Y, Y->X로 연결되지않고 간혹 X->X, Y->Y로 연결되는 현상 방지
		// Euclidean Distance or Manhattan Distance (Optional)
		for (const FVector2D& ATile : RoomATiles)
		{
			for (const FVector2D& BTile : RoomBTiles)
			{
				//CurrentTileDist = FVector2D::DistSquared(ATile, BTile);
				float ManhattanDist = FMath::Abs(ATile.X - BTile.X) + FMath::Abs(ATile.Y - BTile.Y);
				CurrentTileDist = FMath::FloorToInt(ManhattanDist + 0.01f);

				// 가장 짧은 복도 결정
				if (CurrentTileDist < MinTileDist)
				{
					MinTileDist = CurrentTileDist;
					ClosestTileA = ATile;
					ClosestTileB = BTile;
				}
			}
		}
		/*	TODO: Room A, B의 모서리가 평행해 가장 짧은 복도가 여러개일 경우 위와 같은 상황에서는 가장 첫 복도가 계속 선택되어
		*	던전 전체의 시점에서 복도의 생성 위치가 일렬로 편향되는 현상이 발생한다. 이를 위해 ?
		*/

		
		//DrawDebugLine(World, FVector(ClosestTileA, 1750.f), FVector(ClosestTileB, 1750.f), FColor::Blue, true, -1.f, 0, 150.f);

		Thresholds.Push(TPair<FVector2D, FVector2D>(ClosestTileA, ClosestTileB));
	}
	
	/*이제, Thresholds에 복도의 입구 좌표를 쥐고있는 Pair들이 전부 들어있음
	* TODO : Need to PathFind for determine Hallway Grid Block using A* algorithm.
	*/
}

void Floor::PathFind(UWorld* World)
{
	/* 1)우선, Thresholds에 있는 TPair<FVector2D, FVecotr2D>에서 FVector2D의 X, Y 좌표를 int형으로 바꿔줘야한다. 그리드 상태에서 길찾기를 해야하므로,
	* 여기서 중요하게 생각해야할 점은 float형이고 현재 좌표도 잦은 곱셈연산으로 인해 정밀도가 크게 떨어져 있는상황이므로 float + 0.01을 하고 버림을 해야
	* 오차없는 int32형을 얻을 수 있을 것이다.얻은 다음, 길찾기를 하고 TracePath에서 Stack으로부터 목적지 - 출발지를 역추적한 좌표 더미들을 얻을 수 있을텐데,
	* 이 더미들을 TArray에 담아서 또 따로 보관한 뒤, 우선은 DrawDebug를 통해 성공적으로 길찾기가 되었는지 확인하면 될 것이다
	* 
	* ** 또한 길찾기전, grid에 각각의 방 좌표들을 장애물로 넣어줘야한다 여기서, 방의 테두리 부분은 넣지 않도록 한다
	* -> 이미 Threshold들이 테두리 위치로 잡혀있으므로...
	* 
	* TODO : AStar에서 TracePath 수정필요, Stack -> TArray 변환, 
	*/

	/* AStar Exercise */
	TUniquePtr<AStar> PathFind(new AStar());

	UpdatePathFindGrid(World);

	for (const auto& i : Thresholds)
	{
		// ★ TPair<Y, X> ****
		TPair<int32, int32> StartTPair = TPair<int32, int32>(FMath::FloorToInt(i.Key.Y / UnitLength + 0.01f), FMath::FloorToInt(i.Key.X / UnitLength + 0.01f));
		TPair<int32, int32> DestTPair = TPair<int32, int32>(FMath::FloorToInt(i.Value.Y / UnitLength + 0.01f), FMath::FloorToInt(i.Value.X / UnitLength + 0.01f));
		
		PathFind->AStarSearch(PathFindGrid, StartTPair, DestTPair);
		HallwayTrace = PathFind->GetHallwayTrace();
		// Empties array.
		PathFind->ClearHallwayTrace();

		UE_LOG(LogTemp, Warning, TEXT("HallwayTrace's Size : %d"), HallwayTrace.Num());
		
		// reflect Trace of Hallway to PathFindGrid
		for (const TPair<int32, int32>& t : HallwayTrace)
		{
			PathFindGrid[t.Key][t.Value] = 2;
			
			HallwayDraw.Push(t);

			if (HallwayDraw.Num() == 2)
			{
				DrawDebugLine(World, 
					FVector(HallwayDraw[0].Value * UnitLength, HallwayDraw[0].Key * UnitLength, 1800.f), 
					FVector(HallwayDraw[1].Value * UnitLength, HallwayDraw[1].Key * UnitLength, 1800.f), 
					FColor::Turquoise, true, -1.f, 0, 200.f);

				// 0부터 1개 삭제
				if (HallwayDraw.IsValidIndex(0))
					HallwayDraw.RemoveAt(0);
			}
			

			//UE_LOG(LogTemp, Warning, TEXT("%d %d"), t.Key, t.Value);
		}
		//UE_LOG(LogTemp, Warning, TEXT("---------"));
		
		HallwayDraw.Empty();
	}

	// TODO : 출발지 또는 도착지가 Blocked인 예외 상황 발생	
	
	// ExampleGrid를 추후 Floor에서 받아온 Grid 지도로 변경 후 전달한다, 또한 길찾기 후 TArray를 다시 Floor에 넘겨준다
	
	/* 현재까지 확인된 Issue 목록
	*  1) UpdatePathFindGrid로 장애물이 되었어야할 방의 끝부분을 복도 일부가 드물게 지나가는 현상 -> UpdatepPathFindGrid또는 PathFindGrid 배열 확인
	*  2) 복도를 만들고 복도를 이동 불가능하게 변경했는데, 이로 인해 복도가 드물게 겹쳐져 길찾기에 실패하는 현상 (Start or Destination Blocked)
	*  3) Manhattan 거리를 위해 ATiles는 X, Y순으로 거리를 재는 끝부분 배열 추가, BTiles는 반대로 Y, X순으로 해서 의도대로 구현됐으나
	*  Manhattan 거리도 결국 float 형이어서 여러개의 같은 거리가 < 로 여러번 비교되면, flaot 오차로 X->Y, Y->X로 복도가 항상 연결되지 않아야한다고
	*  예측한 것이 실패
	*  -> 3은 해결됐으나 AStar 차원에서 ㄱ자로 연결되면 이상적인데 ㄴ자로 길찾는 현상
	*  -> Manhattan에서 다시 Euclidean으로 변경?
	*  Manhattan은 복도가 단순하다는 장점이 있으나 복도 생성이 너무 유연하지 못함.
	*  ? : AStar의 길찾기 방향을 conditional하게 지정할 순 없을까?
	*  condition을 정하는 것또한 쉽지 않음
	*/


}

void Floor::UpdatePathFindGrid(UWorld* World)
{
	// Initialize Room Coord in Grid
	/*■■■■■■■■■■■
	* ■□□□□□□□□□■
	* ■□□□□□□□□□■
	* ■□□□□□□□□□■
	* ■■■■■■■■■■■
	*/

	// 방마다 내부 x, y 좌표를 Grid에다가 반영, 구조상 3중 loop 불가피
	for (const auto& i : RoomCandidates)
	{
		for (int32 y = i->GetCornerCoordinates().RoomUpperLeftY + 1; y < i->GetCornerCoordinates().RoomLowerRightY - 1; y++)
		{
			for (int32 x = i->GetCornerCoordinates().RoomUpperLeftX + 1; x < i->GetCornerCoordinates().RoomLowerRightX - 1; x++)
			{
				PathFindGrid[y][x] = 0;
			}
		}
	}
	/*for (int32 y = 0; y < FloorGridSizeY; y++)
	{
		for (int32 x = 0; x < FloorGridSizeX; x++)
		{
			if (PathFindGrid[y][x] == 1)
			{
				DrawDebugPoint(World, FVector(x * UnitLength + 250.f, y * UnitLength + 250.f, 2000.f), 10.f, FColor::Emerald, true, -1.f, 0);
			}
		}
	}*/
}
