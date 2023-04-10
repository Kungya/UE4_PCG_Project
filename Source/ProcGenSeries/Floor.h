#pragma once
#include "FloorNode.h"
#include "Room.h"
#include "DelaunayTriangulation.h"

enum class ESplitOrientation
{
	ESO_Horizontal,
	ESO_Vertical
};


class Floor
{
public:
	Floor();
	~Floor();

	void Partition();
	int32 CoinFlip();
	bool ShouldSplitNode(TSharedPtr<FloorNode> InNode, ESplitOrientation Orientation);
	bool SplitAttempt(TSharedPtr<FloorNode> InNode);
	// split A into B and C
	void SplitHorizontal(TSharedPtr<FloorNode> InA, TSharedPtr<FloorNode> InB, TSharedPtr<FloorNode> InC);
	void SplitVertical(TSharedPtr<FloorNode> InA, TSharedPtr<FloorNode> InB, TSharedPtr<FloorNode> InC);

	void DrawFloorNodes(class UWorld* World);
	void DrawFloorNode(UWorld* World, FCornerCoordinates Coordinates, int32 count);

	void SelectRoomCandiate(UWorld* World);
	
	void DrawRoomNodes(UWorld* World);

	void SpawnRoom(UWorld* World);

	void Triangulation(UWorld* World);


	FORCEINLINE TArray<TSharedPtr<FloorNode>> GetPartitionedFloor() const { return PartitionedFloor; }
	FORCEINLINE TArray<TSharedPtr<FloorNode>> GetRoomCandidates() const { return RoomCandidates; }
private:
	TArray<TSharedPtr<class FloorNode>> FloorNodeStack;
	TArray<TSharedPtr<FloorNode>> PartitionedFloor;
	TArray<TSharedPtr<FloorNode>> RoomCandidates;

	/* *★*TODO : 추후 vector들의 메모리 해제를 위한 기능을 만들거나 Triangulation.h내의 
		vector와 그에 대한 함수들을 전부 TArray 계열로 변경해야함.
	*/
	std::vector<delaunay::Point<float>> RoomPointsArr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Room, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ARoom> Room;

	int32 FloorGridSizeX;
	int32 FloorGridSizeY;
	int32 RoomMinX;
	int32 RoomMinY;

	int32 RoomMinArea;

	float UnitLength;

	float SplitChance;
};