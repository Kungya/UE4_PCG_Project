#pragma once

struct FCornerCoordinates
{
	int32 UpperLeftX;
	int32 UpperLeftY;
	int32 LowerRightX;
	int32 LowerRightY;

	// 축소된 이후의 최종적인 Room의 Corner 좌표
	int32 RoomUpperLeftX;
	int32 RoomUpperLeftY;
	int32 RoomLowerRightX;
	int32 RoomLowerRightY;
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
	FORCEINLINE int32 GetArea() const
	{
		int32 NodeWidth = CornerCoordinates.LowerRightX - CornerCoordinates.UpperLeftX;
		int32 NodeLength = CornerCoordinates.LowerRightY - CornerCoordinates.UpperLeftY;
		int32 Area = NodeWidth * NodeLength;

		return Area;
	}
	FORCEINLINE int32 GetWidth() const
	{
		int32 Width = CornerCoordinates.LowerRightX - CornerCoordinates.UpperLeftX;
		
		return Width;
	}
	FORCEINLINE int32 GetLength() const
	{
		int32 Length = CornerCoordinates.LowerRightY - CornerCoordinates.UpperLeftY;

		return Length;
	}
	FORCEINLINE int32 GetRoomWidth() const
	{
		int32 RoomWidth = CornerCoordinates.RoomLowerRightX - CornerCoordinates.RoomUpperLeftX;

		return RoomWidth;
	}

	FORCEINLINE int32 GetRoomLength() const
	{
		int32 RoomLength = CornerCoordinates.RoomLowerRightY - CornerCoordinates.RoomUpperLeftY;

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

	FCornerCoordinates TryGetCornerCoordinatesByMidPoint(float MidPointX, float MidPointY) const;

private:
	FCornerCoordinates CornerCoordinates;

	static int32 FloorNodeCount;

};