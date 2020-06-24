// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chunk.generated.h"

UCLASS()
class MINEUE4_API AChunk : public AActor
{
	GENERATED_BODY()

public:
	static const uint16 CHUNKSIZEX = 16;
	static const uint16 CHUNKSIZEY = 16;
	static const uint16 CHUNKSIZEZ = 32;
	
public:	
	// Sets default values for this actor's properties
	AChunk();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** "On receive chunk data" logic. Client-side*/
	UFUNCTION()
	void OnRep_CompressedBlocks();

private:

	UFUNCTION()
	void UpdateCompressedBlocks();

	UPROPERTY(ReplicatedUsing = OnRep_CompressedBlocks)
	TArray<int64> m_CompressedBlocks;

	UPROPERTY()
	TArray<int64>	m_AllBlocks;

	/*UPROPERTY()
	TArray<int64>	m_AllBlocks;*/
};
