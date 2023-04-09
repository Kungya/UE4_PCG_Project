#pragma once
#include "FloorNode.h"
#include "Room.h"

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

	FORCEINLINE TArray<TSharedPtr<FloorNode>> GetPartitionedFloor() const { return PartitionedFloor; }
	FORCEINLINE TArray<TSharedPtr<FloorNode>> GetRoomCandidates() const { return RoomCandidates; }
private:
	TArray<TSharedPtr<class FloorNode>> FloorNodeStack;
	TArray<TSharedPtr<FloorNode>> PartitionedFloor;
	TArray<TSharedPtr<FloorNode>> RoomCandidates;

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