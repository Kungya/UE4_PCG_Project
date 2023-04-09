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

	RoomMinArea = 80;

	// 실제 길이와 면적은 UnitLength에 곱하여 결정 됨
	// 그러므로 GetArea등에서 이용되는 면적은 UnitLength에 곱해지기 전의 값.
	UnitLength = 500.f;

	SplitChance = 1.75f;


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
		DrawFloorNode(World, Coordinates, i);
	}
}

void Floor::DrawFloorNode(UWorld* World, FCornerCoordinates Coordinates, int32 count)
{
	// count * 10 in Z axis : to distinguish by height
	const FVector UpperLeft(Coordinates.UpperLeftX * UnitLength, Coordinates.UpperLeftY * UnitLength, count * 10);
	const FVector UpperRight(Coordinates.LowerRightX * UnitLength, Coordinates.UpperLeftY * UnitLength, count * 10);
	const FVector LowerLeft(Coordinates.UpperLeftX * UnitLength, Coordinates.LowerRightY * UnitLength, count * 10);
	const FVector LowerRight(Coordinates.LowerRightX * UnitLength, Coordinates.LowerRightY * UnitLength, count * 10);

	
	DrawDebugLine(World, UpperLeft, UpperRight, FColor::Red, true, -1, 0, 50.f);
	DrawDebugLine(World, UpperLeft, LowerLeft, FColor::Red, true, -1, 0, 50.f);
	DrawDebugLine(World, LowerRight, UpperRight, FColor::Red, true, -1, 0, 50.f);
	DrawDebugLine(World, LowerRight, LowerLeft, FColor::Red, true, -1, 0, 50.f);
}

void Floor::SelectRoomCandiate(UWorld* World)
{
	// 1) 일정 조건(크기 80 이상)을 걸어 조건을 만족하는 Floor들을 Patitioned Floor 배열에서 골라낸다
	// 2) 골라낸 새로운 배열에서 골라서 방을 생성하기 전, 크기를 그대로 설정할 경우 인접한 방과의 겹침이
	// 발생할 수 있으니 크기를 1씩 줄인다. -> 1:9, 9:1 내분점을 만들어 각각의 길이를 20%줄여 이전 면적의 64%로 축소하기로 함
	// ** 이 과정에서 방 크기가 사라지거나 심하게 작아질 수 있으므로 이전 단계에서 
	// 너무 작은 방들은 전부 제외하고 중하위 크기 이상의 방들만 골라내는 것 -> 일정 크기 이상의 방이어야 플레이 가능함
	// 기존의 80%로 만든다, 그럼 상하좌우 각각 10%씩 감소시키면 되므로
	// 중점을 구하는 방법과 비슷하게 양끝단 좌표를 더하고 0.1 곱하면 좌측상단 기존의 90%에 해당하는 좌표 생성...
	for (int32 i = 0; i < PartitionedFloor.Num(); i++)
	{
		if (PartitionedFloor[i]->GetArea() > RoomMinArea)
		{
			FCornerCoordinates Coordinates = PartitionedFloor[i]->GetCornerCoordinates();

			// 각각 1:9, 9:1로 내분
			Coordinates.RoomUpperLeftX = (Coordinates.UpperLeftX * 9 + Coordinates.LowerRightX) / 10.f;
			Coordinates.RoomUpperLeftY = (Coordinates.UpperLeftY * 9 + Coordinates.LowerRightY) / 10.f;
			Coordinates.RoomLowerRightX = (Coordinates.UpperLeftX + Coordinates.LowerRightX * 9) / 10.f;
			Coordinates.RoomLowerRightY = (Coordinates.UpperLeftY + Coordinates.LowerRightY * 9) / 10.f;

			PartitionedFloor[i]->SetCornerCoordinates(Coordinates);

			RoomCandidates.Push(PartitionedFloor[i]);
		}
	}
}

void Floor::DrawRoomNodes(UWorld* World)
{
	//생성된 FloorNode들의 중점에 크기 표시
	for (int32 i = 0; i < RoomCandidates.Num(); i++)
	{
		FVector MidPoint(RoomCandidates[i]->GetMidPointX() * UnitLength, RoomCandidates[i]->GetMidPointY() * UnitLength, 10.f);
		DrawDebugString(World, MidPoint, FString::FromInt(RoomCandidates[i]->GetArea()), 0, FColor::Orange, -1.f, false, 3.f);
		DrawDebugString(World, MidPoint + FVector(500.f, 500.f, 0.f), FString::SanitizeFloat(RoomCandidates[i]->GetRoomArea()), 0, FColor::Orange, -1.f, false, 3.f);
		UE_LOG(LogTemp, Warning, TEXT(" RoomArea : %f"), RoomCandidates[i]->GetRoomArea());
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
	}
}