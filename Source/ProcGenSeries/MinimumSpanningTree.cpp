#include "MinimumSpanningTree.h"

MinimumSpanningTree::MinimumSpanningTree(TArray<Node> Nodes, TArray<delaunay::Edge<float>> EdgesArr)
{
	this->Nodes = Nodes;

	hasBeenCalled = false;
	
	for (auto& i : Nodes)
	{
		UE_LOG(LogTemp, Warning, TEXT("Node info : %d %f %f"), i.id, i.x, i.y);
	}
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
	// *** must be called "Only Once" ***
	if (hasBeenCalled)
	{
		UE_LOG(LogTemp, Warning, TEXT("prim() already has been Called, ***WARNING"));
		return NULL;
	}

	// priority_queue를 써야하는데, TPriorityQueue 사용
	// Priority Queue - definition in PriorityQueue.h
	TPriorityQueue<TPair<float, int32>> PQ;
	float result = 0.f;
	// 0번 id부터 시작

	PQ.Push(TPair<float, int32>{ 0.f, 0 });
	arr.Add(PQ.Top().Value);
	/*
	이렇게 정해진다.
	0번 id부터 시작해서, 어디로 갈지는 PQ.Top()의 값에 따라 달려있다.
	그러므로, i=0일 때 간선의 출발지로서 id : 0을 처음 저장하고, 그다음 계속 PQ.Top()을
	*/

	// PQ : { first, second }, first(Key) : Distance, second(Value) : Destination
	// loop for Nodes.Num()
	for (int32 i = 0; i < Nodes.Num(); i++)
	{
		while (!PQ.Empty() && visited[PQ.Top().Value])
		{ // 방문 했었다면 Pop();
			PQ.Pop();
		}

		// PQ.Top() : 최소 가중치를 갖는 목적지, 매 loop 마다 따로 담아서 Floor.cpp에 돌려주면 됨
		// 여기서 PQ에 담겨져있는 정점들은 전부 id이므로, x, y좌표로 변환해주는 작업이 또 다시 필요함
		/* ------------------------------------------------ */
		// i = 0일 때, 첫 출발점이므로 출발지만 추가
		// I >= 1일 때, 목적지를 추가함과 동시에 출발지 추가
		// TODO : Refactoring, 간선을 더 쉽게 저장하는법... 
		if (i > 0)
		{
			arr.Add(PQ.Top().Value);
			MSTEdges.Emplace(TPair<FVector2D, FVector2D>{ ConvertIdToCoord(arr[0]), ConvertIdToCoord(arr[1]) });
			arr.Empty();
			arr.Add(PQ.Top().Value);
		}

		/* ------------------------------------------------ */
		int32 next = PQ.Top().Value; // 목적지
		float minDist = PQ.Top().Key; // 가중치

		visited[next] = true;
		result += minDist;

		for (auto& e : Edge[next])
		{
			PQ.Push(TPair<float, int32>{ e.Value, e.Key });
		}
	}

	while (!PQ.Empty())
	{
		UE_LOG(LogTemp, Warning, TEXT("%f, %d"), PQ.Pop().Key, PQ.Pop().Value);
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

		// search id From Coord of p0(Delaunay edge's vertex), Expect get id.
		for (int32 j = 0; j < Nodes.Num(); j++)
		{
			From_id = Nodes[j].TryGetIdByCoord(p0_x, p0_y);
			if (From_id != -1) // succeed
				break;
		}
		// search id To Coord of p1(Delaunay edge's vertex), Expect get id.
		for (int32 j = 0; j < Nodes.Num(); j++)
		{
			To_id = Nodes[j].TryGetIdByCoord(p1_x, p1_y);
			if (To_id != -1) // succeed
				break;
		}

		UE_LOG(LogTemp, Warning, TEXT("From : %d, To : %d, Dist : %f"), From_id, To_id, Dist);

		Edge[From_id].Emplace(TPair<int32, float>{ To_id, Dist });
		Edge[To_id].Emplace(TPair<int32, float>{ From_id, Dist });
	}
}

FVector2D MinimumSpanningTree::ConvertIdToCoord(const int32& id)
{	/*
		id를 좌표로 변환, TODO : 여기서 추후 과정 진행중, A* 알고리즘 등에서 MST의 정점 좌표로 
		FVector를 요구할지, 또는 Node 구조체로 리턴해서 갖고있는게 더 편할지 모르므로 수정해줘야함. 
		우선은 TArray<Node>와 TArray<FVector> 둘다 생각 해두고 있자.
	*/
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
