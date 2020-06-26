// Fill out your copyright notice in the Description page of Project Settings.


#include "Block.h"
#include "Net/UnrealNetwork.h"

bool UBlock::IsSupportedForNetworking() const
{
  return true;
}

void UBlock::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(UBlock, BlockType);
  DOREPLIFETIME(UBlock, RelativeLocation);
}
