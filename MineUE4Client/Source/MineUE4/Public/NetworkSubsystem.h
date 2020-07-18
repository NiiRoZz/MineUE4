// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Windows/PreWindowsApi.h"
#include "enet/enet.h"
#include "Windows/PostWindowsApi.h"

#include "NetworkSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MINEUE4_API UNetworkSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:

	static UNetworkSubsystem* GetNetworkInstance();
	
public:

	void Initialize(FSubsystemCollectionBase& Collection) override;

	void Deinitialize() override;

	void Tick(float DeltaTime) override;
	bool IsTickableInEditor() const override;
	TStatId GetStatId() const override;
	ETickableTickType GetTickableTickType() const override;


protected:

	ENetHost* m_Host;
};
