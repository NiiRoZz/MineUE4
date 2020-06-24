// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AChunk::AChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_AllBlocks.Reserve(CHUNKSIZEX * CHUNKSIZEY * CHUNKSIZEZ);
}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	Super::BeginPlay();

	SetReplicates(true);
	SetNetDormancy(ENetDormancy::DORM_DormantAll);
	NetCullDistanceSquared = 368640000.f;
	
	//Only at server side
	if (GetLocalRole() == ENetRole::ROLE_Authority)
	{
		for (int x = 0; x < CHUNKSIZEX; ++x)
		{
			for (int y = 0; y < CHUNKSIZEY; ++y)
			{
				for (int z = 0; z < CHUNKSIZEZ; ++z)
				{
					m_AllBlocks[(x * CHUNKSIZEY + y) * CHUNKSIZEZ + z] = 1;
				}
			}
		}

		UpdateCompressedBlocks();

		FlushNetDormancy();
	}
}

// Called every frame
void AChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Only at server side
	if (GetLocalRole() == ENetRole::ROLE_Authority)
	{

	}
}

void AChunk::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AChunk, m_CompressedBlocks);
}

void AChunk::OnRep_CompressedBlocks()
{
	UE_LOG(LogTemp, Warning, TEXT("OnRep_CompressedBlocks"));

	for (int x = 0; x < CHUNKSIZEX; ++x)
	{
		for (int y = 0; y < CHUNKSIZEY; ++y)
		{
			for (int z = 0; z < CHUNKSIZEZ; ++z)
			{
				m_AllBlocks[(x * CHUNKSIZEY + y) * CHUNKSIZEZ + z] = 1;
			}
		}
	}
}

void AChunk::UpdateCompressedBlocks()
{

}

