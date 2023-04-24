#pragma once
#include "FloorNode.h"
#include "Room.h"
#include "DelaunayTriangulation.h"
#include "MinimumSpanningTree.h"
#include "Containers/Set.h"

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
	void DrawFloorNode(UWorld* World, FCornerCoordinates Coordinates, int32 count, FColor NodeColor);
	void DrawGrid(UWorld* World);

	void SelectRoomCandiate(UWorld* World);
	
	void DrawRoomNodes(UWorld* World);

	void SpawnRoom(UWorld* World);

	void Triangulation(UWorld* World);

	void SetNodes(UWorld* World);

	void CreateMST(UWorld* World);

	/* ------------------------------------------------ */
	FORCEINLINE bool bEdgesEqual(const TPair<FVector2D, FVector2D>& Edge1, const TPair<FVector2D, FVector2D>& Edge2)
	{
		return ((Edge1.Key.Equals(Edge2.Key) && Edge1.Value.Equals(Edge2.Value)) || (Edge1.Key.Equals(Edge2.Value) && Edge1.Value.Equals(Edge2.Key)));
	}

	FORCEINLINE bool AddUniqueEdge(TArray<TPair<FVector2D, FVector2D>>& EdgeArray, const TPair<FVector2D, FVector2D>& newEdge)
	{
		for (auto& Edge : EdgeArray)
		{
			if (bEdgesEqual(Edge, newEdge))
			{
				// 이미 존재하는 복도(간선)
				return false;
			}
		}
		// Add Unqiue 성공
		EdgeArray.Add(newEdge);
		return true;
	}

	void ReincorporateEdge(UWorld* World);
	/* ------------------------------------------------ */

	FORCEINLINE void SetMSTEdges(TArray<TPair<FVector2D, FVector2D>> newMSTEdges) { MSTEdges = newMSTEdges; }

	FORCEINLINE TArray<TSharedPtr<FloorNode>> GetPartitionedFloor() const { return PartitionedFloor; }
	FORCEINLINE TArray<TSharedPtr<FloorNode>> GetRoomCandidates() const { return RoomCandidates; }
	
	FORCEINLINE TArray<TPair<FVector2D, FVector2D>> GetHallways() const { return Hallways; };
private:
	TArray<TSharedPtr<class FloorNode>> FloorNodeStack;
	TArray<TSharedPtr<FloorNode>> PartitionedFloor;
	TArray<TSharedPtr<FloorNode>> RoomCandidates;

	/* *★*TODO : 추후 vector들의 메모리 해제를 위한 기능을 만들거나 Triangulation.h내의 
		vector와 그에 대한 함수들을 전부 TArray 계열로 변경해야함.
	*/
	std::vector<delaunay::Point<float>> RoomPointsArr;

	TArray<delaunay::Edge<float>> TriangulatedUniqueEdgesArr;

	TArray<Node> Nodes;

	TArray<TPair<FVector2D, FVector2D>> MSTEdges;

	TArray<TPair<FVector2D, FVector2D>> Hallways;

	float MinCostSum;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Room, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ARoom> Room;

	int32 FloorGridSizeX;
	int32 FloorGridSizeY;
	int32 RoomMinX;
	int32 RoomMinY;

	int32 RoomMinArea;
	float RoomMinRatio;

	float UnitLength;

	float SplitChance;

	float ReincorporationChance;
};