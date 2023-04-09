#include "FloorNode.h"

int32 FloorNode::FloorNodeCount = 0;

FloorNode::FloorNode()
{
	// To Trace Number of FloorNode
	++FloorNodeCount;

	//UE_LOG(LogTemp, Warning, TEXT("FloorNode Created"));
}

FloorNode::FloorNode(const FCornerCoordinates& Coordinates)
{
	// To Trace Number of FloorNode
	++FloorNodeCount;

	//UE_LOG(LogTemp, Warning, TEXT("FloorNode Created"));

	//CornerCoordinates = Coordinates;

	CornerCoordinates.UpperLeftX = Coordinates.UpperLeftX;
	CornerCoordinates.UpperLeftY = Coordinates.UpperLeftY;
	CornerCoordinates.LowerRightX = Coordinates.LowerRightX;
	CornerCoordinates.LowerRightY = Coordinates.LowerRightY;
}

FloorNode::~FloorNode()
{
	--FloorNodeCount;

	//UE_LOG(LogTemp, Warning, TEXT("FloorNode Destroyed"));
}