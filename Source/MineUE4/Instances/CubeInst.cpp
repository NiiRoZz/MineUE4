// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeInst.h"

// Sets default values
ACubeInst::ACubeInst()
{
  // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = false;

  m_MeshInst = CreateDefaultSubobject<UInstancedStaticMeshComponent>("m_MeshInst");

  RootComponent = m_MeshInst;
}

UInstancedStaticMeshComponent* ACubeInst::GetMeshInst()
{
  return m_MeshInst;
}

// Called when the game starts or when spawned
void ACubeInst::BeginPlay()
{
  Super::BeginPlay();

  SetReplicates(false);
  
  FTransform transform;

  FVector loc(0.0);

  transform.SetLocation(loc);

  //int32 idx = m_MeshInst->AddInstance(transform);
  //m_MeshInst->SetCustomDataValue(idx, 0, 255.f);
}

// Called every frame
void ACubeInst::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

}

