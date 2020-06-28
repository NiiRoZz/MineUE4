// Fill out your copyright notice in the Description page of Project Settings.


#include "Block.h"
#include "Net/UnrealNetwork.h"

bool FBlock::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
  Ar << BlockType;

  Ar.SerializeBits(&(RelativeLocation[0]), 4);
  Ar.SerializeBits(&(RelativeLocation[1]), 4);
  Ar.SerializeBits(&(RelativeLocation[2]), 4);

  bOutSuccess = true;

  return true;
}
