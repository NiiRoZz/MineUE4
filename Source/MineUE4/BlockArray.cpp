// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockArray.h"
#include "Chunks/Chunk.h"

bool FCompressedBlock::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
  static const uint16 NMBBITSMAXTYPEOFBLOCKS = (uint16)FMath::Log2(AChunk::MAXTYPEOFBLOCKS - 1) + 1;
  static const uint32 NMBMAXBBLOCKS = (uint16)FMath::Log2(AChunk::CHUNKSIZEX * AChunk::CHUNKSIZEY * AChunk::CHUNKSIZEZ - 1) + 1;

  //Max is 255 inclusive, so we need to only serialize only 8 bits
  Ar.SerializeBits(&BlockType, NMBBITSMAXTYPEOFBLOCKS);

  Ar.SerializeBits(&NmbBlocks, NMBMAXBBLOCKS);

  bOutSuccess = true;

  return true;
}
