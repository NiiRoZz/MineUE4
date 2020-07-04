// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockArray.h"
#include "Chunk.h"

bool FCompressedBlock::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
  static const int32 NMBBITSMAXTYPEOFBLOCKS = (int32)FMath::Log2(AChunk::MAXTYPEOFBLOCKS - 1) + 1;
  static const int32 NMBMAXBBLOCKS = (int32)FMath::Log2(AChunk::CHUNKSIZEX * AChunk::CHUNKSIZEY * AChunk::CHUNKSIZEZ - 1) + 1;

  //Max is 255 inclusive, so we need to only serialize only 8 bits
  Ar.SerializeBits(&BlockType, NMBBITSMAXTYPEOFBLOCKS);

  Ar.SerializeBits(&NmbBlocks, NMBMAXBBLOCKS);

  bOutSuccess = true;

  return true;
}
