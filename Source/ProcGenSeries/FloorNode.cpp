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

FCornerCoordinates FloorNode::TryGetCornerCoordinatesByMidPoint(float MidPointX, float MidPointY) const
{
	float tolerance = 0.01f;
	//UE_LOG(LogTemp, Error, TEXT("%f == %f | %f == %f"), GetMidPointX(), MidPointX, GetMidPointY(), MidPointY);
	if (FMath::IsNearlyEqual(GetMidPointX(), MidPointX, tolerance) && FMath::IsNearlyEqual(GetMidPointY(), MidPointY, tolerance))
	{ // succeed
		return CornerCoordinates;
	}
	else
	{ // fail
		return FCornerCoordinates{ -1, -1, -1, -1,-1, -1, -1, -1 };
	}
}