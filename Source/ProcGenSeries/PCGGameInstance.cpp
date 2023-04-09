// Fill out your copyright notice in the Description page of Project Settings.


#include "PCGGameInstance.h"

void UPCGGameInstance::Init()
{
}

void UPCGGameInstance::SetGeneratedRoomsArray(TArray<TSharedPtr<FloorNode>> RoomsArray)
{
	GeneratedRooms = RoomsArray;
}
