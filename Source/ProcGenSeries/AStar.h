#pragma once
#include "Floor.h"

// or not use TArray and AddUnique, sort
#include "set"

#define MapGridSizeY 9
#define MapGridSizeX 10

struct cell
{
	int32 ParentY;
	int32 ParentX;

	double f, g, h;
};

class AStar
{
public:
	void TracePath(cell cellDetails[][MapGridSizeX], std::pair<int32, int32> Dest)
	{
		UE_LOG(LogTemp, Error, TEXT("The Path is"));
		int32 Y = Dest.first;
		int32 X = Dest.second;

		TArray<TPair<int32, int32>> Path;

		while (!(cellDetails[Y][X].ParentY == Y && cellDetails[Y][X].ParentX == X))
		{
			Path.Push(TPair<int32, int32>(Y, X));

			int32 tempY = cellDetails[Y][X].ParentY;
			int32 tempX = cellDetails[Y][X].ParentX;
			Y = tempY;
			X = tempX;
		}

		Path.Push(TPair<int32, int32>(Y, X));

		while (!(Path.Num() == 0))
		{
			TPair<int32, int32> p = Path.Top();
			Path.Pop();
			UE_LOG(LogTemp, Error, TEXT("-> ( %d, %d )"), p.Key, p.Value);
		}

		return;
	}

	void AStarSearch(int32 grid[][MapGridSizeX], std::pair<int32, int32> Start = {8, 0}, std::pair<int32, int32> Dest = {0, 0})
	{
		if (IsValid(Start.first, Start.second) == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("Start is invalid"));
			return;
		}

		if (IsValid(Dest.first, Dest.second) == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("Destination is invalid"));
			return;
		}

		if (IsUnBlocked(grid, Start.first, Start.second) == false 
			|| IsUnBlocked(grid, Dest.first, Dest.second) == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("Start or Destination is Blocked"));
			return;
		}

		if (IsDestination(Start.first, Start.second, Dest) == true)
		{
			UE_LOG(LogTemp, Warning, TEXT("Already at the Destination"));
			return;
		}

		// TODO : convert UE Container and memeset
		bool ClosedList[MapGridSizeY][MapGridSizeX];
		memset(ClosedList, false, sizeof(ClosedList));

		cell cellDetails[MapGridSizeY][MapGridSizeX];

		int32 i, j;

		for (i = 0; i < MapGridSizeY; i++)
		{
			for (j = 0; j < MapGridSizeX; j++)
			{
				cellDetails[i][j].g = FLT_MAX;
				cellDetails[i][j].h = FLT_MAX;
				cellDetails[i][j].f = FLT_MAX;
				cellDetails[i][j].ParentY = -1;
				cellDetails[i][j].ParentX = -1;
			}
		}

		i = Start.first, j = Start.second;
		// 출발점은 유일하게 부모가 자기자신임. 추후 TracePath에서 이걸 이용
		cellDetails[i][j].g = 0.f;
		cellDetails[i][j].h = 0.f;
		cellDetails[i][j].f = 0.f;
		cellDetails[i][j].ParentY = i;
		cellDetails[i][j].ParentX = j;

		// TODO : convert set
		std::set<std::pair<double, std::pair<int32, int32>>> OpenList;

		OpenList.insert({ 0.f, {i, j} });

		bool bFoundDest = false;

		while (!OpenList.empty())
		{
			std::pair<double, std::pair<int32, int32>> p = *OpenList.begin();
			OpenList.erase(OpenList.begin());
			i = p.second.first;
			j = p.second.second;
			ClosedList[i][j] = true;

			double Newg, Newh, Newf;

			/* -------- (1) North -------- */
			if (IsValid(i - 1, j) == true)
			{
				if (IsDestination(i - 1, j, Dest) == true)
				{
					cellDetails[i - 1][j].ParentY = i;
					cellDetails[i - 1][j].ParentX = j;
					UE_LOG(LogTemp, Error, TEXT("Destination cell is found"));
					TracePath(cellDetails, Dest);
					return;
				}
				else if (ClosedList[i - 1][j] == false && IsUnBlocked(grid, i - 1, j) == true)
				{
					Newg = cellDetails[i][j].g + 1.f;
					Newh = CalculateHeuristicValue(i - 1, j, Dest);
					Newf = Newg + Newh;

					if (cellDetails[i - 1][j].f == FLT_MAX || cellDetails[i - 1][j].f > Newf)
					{
						OpenList.insert({ Newf, {i - 1, j} });

						cellDetails[i - 1][j].g = Newg;
						cellDetails[i - 1][j].h = Newh;
						cellDetails[i - 1][j].f = Newf;
						cellDetails[i - 1][j].ParentY = i;
						cellDetails[i - 1][j].ParentX = j;
					}
				}
			}

			/* -------- (2) South -------- */
			if (IsValid(i + 1, j) == true)
			{
				if (IsDestination(i + 1, j, Dest) == true)
				{
					cellDetails[i + 1][j].ParentY = i;
					cellDetails[i + 1][j].ParentX = j;
					UE_LOG(LogTemp, Error, TEXT("Destination cell is found"));
					TracePath(cellDetails, Dest);
					return;
				}
				else if (ClosedList[i + 1][j] == false && IsUnBlocked(grid, i + 1, j) == true)
				{
					Newg = cellDetails[i][j].g + 1.f;
					Newh = CalculateHeuristicValue(i + 1, j, Dest);
					Newf = Newg + Newh;

					if (cellDetails[i + 1][j].f == FLT_MAX || cellDetails[i + 1][j].f > Newf)
					{
						OpenList.insert({ Newf, {i + 1 ,j} });
						
						cellDetails[i + 1][j].g = Newg;
						cellDetails[i + 1][j].h = Newh;
						cellDetails[i + 1][j].f = Newf;
						cellDetails[i + 1][j].ParentY = i;
						cellDetails[i + 1][j].ParentX = j;
					}
				}
			}

			/* -------- (3) East -------- */
			if (IsValid(i, j + 1) == true)
			{
				if (IsDestination(i, j + 1, Dest) == true)
				{
					cellDetails[i][j + 1].ParentY = i;
					cellDetails[i][j + 1].ParentX = j;
					UE_LOG(LogTemp, Error, TEXT("Destination cell is found"));
					TracePath(cellDetails, Dest);
					return;
				}
				else if (ClosedList[i][j + 1] == false && IsUnBlocked(grid, i, j + 1) == true)
				{
					Newg = cellDetails[i][j].g + 1.f;
					Newh = CalculateHeuristicValue(i, j + 1, Dest);
					Newf = Newg + Newh;

					if (cellDetails[i][j + 1].f == FLT_MAX || cellDetails[i][j + 1].f > Newf)
					{
						OpenList.insert({ Newf, {i, j + 1} });

						cellDetails[i][j + 1].g = Newg;
						cellDetails[i][j + 1].h = Newh;
						cellDetails[i][j + 1].f = Newf;
						cellDetails[i][j + 1].ParentY = i;
						cellDetails[i][j + 1].ParentX = j;
					}
				}
			}

			/* -------- (4) West -------- */
			if (IsValid(i, j - 1) == true)
			{
				if (IsDestination(i, j - 1, Dest) == true)
				{
					cellDetails[i][j - 1].ParentY = i;
					cellDetails[i][j - 1].ParentX = j;
					UE_LOG(LogTemp, Error, TEXT("Destination cell is found"));
					TracePath(cellDetails, Dest);
					return;
				}
				else if (ClosedList[i][j - 1] == false && IsUnBlocked(grid, i, j - 1) == true)
				{
					Newg = cellDetails[i][j].g + 1.f;
					Newh = CalculateHeuristicValue(i, j - 1, Dest);
					Newf = Newg + Newh;

					if (cellDetails[i][j - 1].f == FLT_MAX || cellDetails[i][j - 1].f > Newf)
					{
						OpenList.insert({ Newf, {i, j - 1} });

						cellDetails[i][j - 1].g = Newg;
						cellDetails[i][j - 1].h = Newh;
						cellDetails[i][j - 1].f = Newf;
						cellDetails[i][j - 1].ParentY = i;
						cellDetails[i][j - 1].ParentX = j;
					}
				}
			}
		}

		if (bFoundDest == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("Fail to find the Destination Cell"));
		}

		return;
	}


private:
	bool IsValid(int32 Y, int32 X)
	{
		return (Y >= 0) && (Y < MapGridSizeY) && (X >= 0) && (X < MapGridSizeX);
	}

	bool IsUnBlocked(int32 grid[][MapGridSizeX], int32 Y, int32 X)
	{
		if (grid[Y][X] == 1)
			return true;
		else
			return false;
	}

	bool IsDestination(int32 Y, int32 X, std::pair<int32, int32> dest)
	{
		if (Y == dest.first && X == dest.second)
			return true;
		else
			return false;
	}

	double CalculateHeuristicValue(int32 Y, int32 X, std::pair<int32, int32> Dest)
	{
		// Using Euc Distance.
		return ((double)sqrt
		((Y - Dest.first) * (Y - Dest.first) + (X - Dest.second) * (X - Dest.second)));
	}
};