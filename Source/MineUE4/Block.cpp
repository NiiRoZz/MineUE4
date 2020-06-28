// Fill out your copyright notice in the Description page of Project Settings.


#include "Block.h"
#include "Chunks/Chunk.h"
#include "Net/UnrealNetwork.h"
#include "Math/UnrealMathUtility.h"

bool FBlock::IsTransparent()
{
  switch (BlockType)
  {
    case 0:
    {
      return true;
    }
  }

  return false;
}

bool FBlock::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
  static const uint16 NMBBITSCUNKSIZEX = (uint16)FMath::Log2(AChunk::CHUNKSIZEX - 1) + 1;
  static const uint16 NMBBITSCUNKSIZEY = (uint16)FMath::Log2(AChunk::CHUNKSIZEY - 1) + 1;
  static const uint16 NMBBITSCUNKSIZEZ = (uint16)FMath::Log2(AChunk::CHUNKSIZEZ - 1) + 1;
  static const uint16 NMBBITSMAXTYPEOFBLOCKS = (uint16)FMath::Log2(AChunk::MAXTYPEOFBLOCKS - 1) + 1;

  //Max is 255 inclusive, so we need to only serialize only 8 bits
  Ar.SerializeBits(&BlockType, NMBBITSMAXTYPEOFBLOCKS);

  Ar.SerializeBits(&(RelativeLocation[0]), NMBBITSCUNKSIZEX);
  Ar.SerializeBits(&(RelativeLocation[1]), NMBBITSCUNKSIZEY);
  Ar.SerializeBits(&(RelativeLocation[2]), NMBBITSCUNKSIZEZ);

  bOutSuccess = true;

  return true;
}
