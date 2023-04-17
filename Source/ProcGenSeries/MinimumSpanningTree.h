#pragma once

#include "DelaunayTriangulation.h"
#include "PriorityQueue.h"
// TODO : 메모리 관리를 위해 컨테이너 변경

// MAX Num of Edge
constexpr auto MAX_N = 101;

struct Node
{
	Node(int i, float x, float y) : id(i), x(x), y(y) {};

	int32 id;
	float x, y;

	// not used yet.
	bool operator==(const Node& otherNode) const
	{
		// TODO : 조건에 id 까지?
		bool bEqualNode = FMath::IsNearlyEqual(x, otherNode.x) && FMath::IsNearlyEqual(y, otherNode.y);
		return bEqualNode;
	}

	// TODO : Get id By Coord
	FORCEINLINE int32 TryGetIdByCoord(float CoordX, float CoordY) const
	{
		float Tolerance = 0.01f;
		if (FMath::IsNearlyEqual(x, CoordX, Tolerance) && FMath::IsNearlyEqual(y, CoordY, Tolerance))
		{ // succeed
			//UE_LOG(LogTemp, Warning, TEXT("id %d : %f %f | %f %f"), id, x, CoordX, y, CoordY);
			return id;
		}
		else
		{ // fail
			return -1;
		}
	}

	FORCEINLINE FVector2D TryGetCoordById(int32 InputId) const
	{
		if (id == InputId)
		{ // succeed
			return FVector2D(x, y);
		}
		else
		{ // fail, TODO : return symbol that can be Expected to be Error
			return FVector2D::ZeroVector;
		}
	}
};


class MinimumSpanningTree
{
public:
	MinimumSpanningTree(TArray<Node> Nodes, TArray<delaunay::Edge<float>> EdgesArr, UWorld* World);

	float GetDistance(float x1, float y1, float x2, float y2);

	void SetEdges();

	float prim();

	void ConvertCoordToIdAndAddEdge(TArray<delaunay::Edge<float>> EdgesArr);

	int32 SearchStartingPointOfEdge(float Dist, int32 DestinationId);

	int32 ConvertCoordToId(const float& CoordX, const float& CoordY);
	FVector2D ConvertIdToCoord(const int32& id);

	FORCEINLINE TArray<TPair<FVector2D, FVector2D>> GetMSTEdges()
	{
		// TODO : about using Move Semantics, except const in function 
		return MoveTemp(MSTEdges);
	}
private:
	TArray<Node> Nodes;
	
	TArray<TPair<int32, float>> Edge[MAX_N];

	bool visited[MAX_N] = { 0, };

	TArray<int32> arr;

	TArray<TPair<FVector2D, FVector2D>> MSTEdges;

	bool hasBeenCalled;

	UWorld* World;

};