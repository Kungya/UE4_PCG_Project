#include "MinimumSpanningTree.h"
#include "DrawDebugHelpers.h"

MinimumSpanningTree::MinimumSpanningTree(TArray<Node> Nodes, TArray<delaunay::Edge<float>> EdgesArr, UWorld* World)
{
	this->Nodes = Nodes;
	this->World = World;

	hasBeenCalled = false;
	
	/*	방의 중점 vertex들과 간선이 저장된 TArray가 다 넘어왔음, 이제 EdgesArr를 까서 연결된 간선 정보를 업데이트하고,
		.... 프림 알고리즘을 짜서 실행한 뒤, 가중치 합의 값을 출력하고 MST에 해당하는 간선들을 넘기면 됨
	*/
	// 이제, Nodes에 정점들의 좌표와 id가 들어가 있고, Edge 에도 연결된 간선과 가중치 정보가 다 들어가 있다 

	ConvertCoordToIdAndAddEdge(EdgesArr);
}

float MinimumSpanningTree::GetDistance(float x1, float y1, float x2, float y2)
{
	float dx = (x2 - x1) * (x2 - x1);
	float dy = (y2 - y1) * (y2 - y1);
	float distance = FMath::Sqrt(dx + dy);

	return distance;
}

void MinimumSpanningTree::SetEdges()
{

}


float MinimumSpanningTree::prim()
{
	if (hasBeenCalled)
	{
		UE_LOG(LogTemp, Warning, TEXT("prim() already has been Called, ***WARNING"));
		return NULL;
	}

	TPriorityQueue<TPair<float, int32>> PQ;
	
	float result = 0.f;
	PQ.Push(TPair<float, int32>(0.f, 0));

	// 여기서 PQ에 담겨져있는 정점들은 전부 id이므로, x, y좌표로 변환해주는 작업이 또 다시 필요함
	// PQ : { first, second }, first(Key) : Distance, second(Value) : Destination
	// loop for Nodes.Num()
	for (int32 i = 0; i < Nodes.Num(); i++)
	{
		while (!PQ.Empty() && visited[PQ.Top().Value])
		{ // 방문 했었다면 Pop();
			PQ.Pop();
		}

		/* ------------------------------------------------ */
		if (i > 0)
		{ 
			// PQ.Top().Value : 4, Top()에는 4854, 4라는 정보만 있다, 여기로 가야하는데 출발지를 찾아야함, 4854가중치와 목적지 4를 갖는 출발지 값을 어디서찾아야할까?->edge
			arr.Add(SearchStartingPointOfEdge(PQ.Top().Key, PQ.Top().Value));
			arr.Add(PQ.Top().Value);

			// TODO(ERROR) : 2-300번에 한번씩 변환이 실패해 MST의 간선 중 정점 하나가 다른 정점으로 변환되는 현상이 발견됨
			MSTEdges.Emplace(TPair<FVector2D, FVector2D>(ConvertIdToCoord(arr[0]), ConvertIdToCoord(arr[1])));
			if (arr.Num() == 2)
			{
				// 2개 삭제
				arr.RemoveAt(0, 2);
			}
		}
		/* ------------------------------------------------ */
		int32 next = PQ.Top().Value; // 목적지
		float minDist = PQ.Top().Key; // 가중치

		visited[next] = true;
		result += minDist;

		for (auto& e : Edge[next])
		{
			if (!visited[e.Key])
				PQ.Push(TPair<float, int32>(e.Value, e.Key));
		}
	}
	// 알고리즘이 끝나고 안전하게 전부 Pop()
	while (!PQ.Empty())
	{
		PQ.Pop();
	}

	hasBeenCalled = true;
	return result;
}

void MinimumSpanningTree::ConvertCoordToIdAndAddEdge(TArray<delaunay::Edge<float>> EdgesArr)
{
	for (int32 i = 0; i < EdgesArr.Num(); i++)
	{
		int32 From_id, To_id;
		float p0_x = EdgesArr[i].p0.x;
		float p0_y = EdgesArr[i].p0.y;
		float p1_x = EdgesArr[i].p1.x;
		float p1_y = EdgesArr[i].p1.y;
		float Dist = GetDistance(p0_x, p0_y, p1_x, p1_y);

		// search id From Coord of p0 and p1(Delaunay edge's vertex), Expect get id.
		From_id = ConvertCoordToId(p0_x, p0_y);
		To_id = ConvertCoordToId(p1_x, p1_y);

		Edge[From_id].Emplace(TPair<int32, float>(To_id, Dist));
		Edge[To_id].Emplace(TPair<int32, float>(From_id, Dist));
	}
}

int32 MinimumSpanningTree::SearchStartingPointOfEdge(float Dist, int32 DestinationId)
{
	float tolerance = 0.001f;
	// 가중치와 목적지 id가 주어지면, edge 배열에서 탐색해서 출발지를 반환
	// -> prim() 알고리즘 loop 진행중, 각 간선이 만들어질 때마다 우선순위 큐에서는 Top()에서 가중치와 목적지만 있으므로,
	// 출발지를 탐색해야 간선을 생성하고 연결할 수 있기 때문에 이 과정을 거치는 것임.
	for (int32 i = 0; i < MAX_N; i++)
	{
		for (int32 j = 0; j < Edge[i].Num(); j++)
		{
			if (Edge[i][j].Key == DestinationId && FMath::IsNearlyEqual(Edge[i][j].Value, Dist, tolerance))
			{
				return i;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Search Error !!!!!!!!!!!!!!!!!!!!!!!"));
	return -1;
}

int32 MinimumSpanningTree::ConvertCoordToId(const float& CoordX, const float& CoordY)
{
	int32 ConvertedId = -1;

	for (int32 i = 0; i < Nodes.Num(); i++)
	{
		ConvertedId = Nodes[i].TryGetIdByCoord(CoordX, CoordY);
		if (ConvertedId != -1) // succeed
			break;
	}

	return ConvertedId;
}

FVector2D MinimumSpanningTree::ConvertIdToCoord(const int32& id)
{	/*	id를 좌표로 변환, TODO : 여기서 추후 과정 진행중, A* 알고리즘 등에서 MST의 정점 좌표로 
		FVector를 요구할지, 또는 Node 구조체로 리턴해서 갖고있는게 더 편할지 모르므로 수정해줘야함. 
		우선은 TArray<Node>와 TArray<FVector> 둘다 생각 해두고 있자.	*/
	FVector2D MSTCoord;

	// search Coord From id, Expect get Coord.
	for (int32 i = 0; i < Nodes.Num(); i++)
	{
		MSTCoord = Nodes[i].TryGetCoordById(id);
		if (!MSTCoord.IsZero())
			break;
	}

	return MSTCoord;
}
