#pragma once

template <typename InElementType>
struct TPriorityQueueNode
{
	InElementType Element;
	InElementType Priority;

	TPriorityQueueNode() {}

	// Expect automatically set Priority by InElement
	TPriorityQueueNode(InElementType InElement)
	{
		Element = InElement;
		Priority = InElement;
	}

	// min to max
	bool operator<(const TPriorityQueueNode<InElementType> Other) const
	{
		return Priority < Other.Priority;
	}
};

template <typename InElementType>
class TPriorityQueue
{
public:
	TPriorityQueue()
	{ // convert array to heap
		Array.Heapify();
	}

public:
	// ** Always check if IsEmpty() before Pop !
	InElementType Pop()
	{
		TPriorityQueueNode<InElementType> Node;
		Array.HeapPop(Node);

		return Node.Element;
	}
	TPriorityQueueNode<InElementType> PopNode()
	{
		TPriorityQueueNode<InElementType> Node;
		Array.HeapPop(Node);

		return Node;
	}

	void Push(InElementType Element)
	{
		Array.HeapPush(TPriorityQueueNode<InElementType>(Element));
	}

	bool Empty() const
	{
		return Array.Num() == 0;
	}

	InElementType Top()
	{
		return Array.HeapTop().Element;
	}

private:
	TArray<TPriorityQueueNode<InElementType>> Array;
};