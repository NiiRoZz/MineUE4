// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeInst.h"

// Sets default values
ACubeInst::ACubeInst()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_MeshInst = CreateDefaultSubobject<UInstancedStaticMeshComponent>("m_MeshInst");

	RootComponent = m_MeshInst;
}

// Called when the game starts or when spawned
void ACubeInst::BeginPlay()
{
	Super::BeginPlay();
	
	FTransform transform;

	FVector loc(0.0);

	transform.SetLocation(loc);

	m_MeshInst->AddInstance(transform);
}

// Called every frame
void ACubeInst::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

