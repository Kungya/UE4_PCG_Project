#pragma once

struct FCornerCoordinates
{
	int32 UpperLeftX;
	int32 UpperLeftY;
	int32 LowerRightX;
	int32 LowerRightY;

	// 축소된 이후의 최종적인 Room의 Corner 좌표
	float RoomUpperLeftX;
	float RoomUpperLeftY;
	float RoomLowerRightX;
	float RoomLowerRightY;
};

class FloorNode
{
public:
	FloorNode();
	FloorNode(const FCornerCoordinates& Coordinates);

	~FloorNode();

	FORCEINLINE FCornerCoordinates GetCornerCoordinates() const { return CornerCoordinates; }
	FORCEINLINE void SetCornerCoordinates(FCornerCoordinates Coordinates) { CornerCoordinates = Coordinates; }

	FORCEINLINE static int32 GetNodeCount() { return FloorNodeCount; }
	FORCEINLINE int32 GetArea() const // Floor.cpp내 for문에서 호출될 것이므로 FORCEINLINE 선택
	{
		int32 NodeWidth = CornerCoordinates.LowerRightX - CornerCoordinates.UpperLeftX;
		int32 NodeLength = CornerCoordinates.LowerRightY - CornerCoordinates.UpperLeftY;
		int32 Area = NodeWidth * NodeLength;

		return Area;
	}
	FORCEINLINE float GetWidth() const
	{
		float Width = CornerCoordinates.LowerRightX - CornerCoordinates.UpperLeftX;
		
		return Width;
	}
	FORCEINLINE float GetLength() const
	{
		float Length = CornerCoordinates.LowerRightY - CornerCoordinates.UpperLeftY;

		return Length;
	}
	FORCEINLINE float GetRoomWidth() const
	{
		float RoomWidth = CornerCoordinates.RoomLowerRightX - CornerCoordinates.RoomUpperLeftX;

		return RoomWidth;
	}

	FORCEINLINE float GetRoomLength() const
	{
		float RoomLength = CornerCoordinates.RoomLowerRightY - CornerCoordinates.RoomUpperLeftY;

		return RoomLength;
	}

	FORCEINLINE float GetFloorNodeRatio() const
	{
		float Width = GetWidth();
		float Length = GetLength();

		if (Width < Length)
			return Width / Length;
		else
			return Length / Width;
	}

	FORCEINLINE float GetRoomArea() const // 축소된 이후의 Area
	{
		float RoomWidth = CornerCoordinates.RoomLowerRightX - CornerCoordinates.RoomUpperLeftX;
		float RoomLength = CornerCoordinates.RoomLowerRightY - CornerCoordinates.RoomUpperLeftY;

		return RoomWidth * RoomLength;
	}

	FORCEINLINE FVector GetRoomSize() const
	{
		float XSize = CornerCoordinates.RoomLowerRightX - CornerCoordinates.RoomUpperLeftX;
		float YSize = CornerCoordinates.RoomLowerRightY - CornerCoordinates.RoomUpperLeftY;
		FVector RoomSize = FVector(XSize, YSize, 0.f);

		return RoomSize;
	}


	FORCEINLINE float GetMidPointX() const
	{
		return (CornerCoordinates.RoomUpperLeftX + CornerCoordinates.RoomLowerRightX) / 2.f;
	}
	FORCEINLINE float GetMidPointY() const
	{
		return (CornerCoordinates.RoomUpperLeftY + CornerCoordinates.RoomLowerRightY) / 2.f;
	}

	//FCornerCoordinates TryGetGridCoordByMidPoint(float MidPointX, float MidPointY) const;

private:
	FCornerCoordinates CornerCoordinates;

	static int32 FloorNodeCount;

};