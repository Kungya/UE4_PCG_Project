// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FloorNode.h"
#include "Engine/GameInstance.h"
#include "PCGGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROCGENSERIES_API UPCGGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	void SetGeneratedRoomsArray(TArray<TSharedPtr<FloorNode>> RoomsArray);

private:
	TArray<TSharedPtr<FloorNode>> GeneratedRooms;
};
