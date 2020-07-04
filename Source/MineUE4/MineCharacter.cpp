// Fill out your copyright notice in the Description page of Project Settings.


#include "MineCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Chunks/Chunk.h"
#include "Chunks/ChunkManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMineCharacter::AMineCharacter()
{
  // Set size for collision capsule
  GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

  // set our turn rates for input
  BaseTurnRate = 45.f;
  BaseLookUpRate = 45.f;

  // Create a CameraComponent	
  FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
  FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
  FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
  FirstPersonCameraComponent->bUsePawnControlRotation = true;
}

// Called when the game starts or when spawned
void AMineCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMineCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMineCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

  // set up gameplay key bindings
  check(PlayerInputComponent);

  // Bind jump events
  PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
  PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

  // Bind movement events
  PlayerInputComponent->BindAxis("MoveForward", this, &AMineCharacter::MoveForward);
  PlayerInputComponent->BindAxis("MoveRight", this, &AMineCharacter::MoveRight);

  // We have 2 versions of the rotation bindings to handle different kinds of devices differently
  // "turn" handles devices that provide an absolute delta, such as a mouse.
  // "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
  PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
  PlayerInputComponent->BindAxis("TurnRate", this, &AMineCharacter::TurnAtRate);
  PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
  //PlayerInputComponent->BindAxis("LookUpRate", this, &AFirstPersonCharacter::LookUpAtRate);

  //Bind block events
  PlayerInputComponent->BindAction("BreakBlock", IE_Pressed, this, &AMineCharacter::BreakBlockClient);
  PlayerInputComponent->BindAction("PlaceBlock", IE_Pressed, this, &AMineCharacter::PlaceBlockClient);

}

void AMineCharacter::MoveForward(float Value)
{
  if (Value != 0.0f)
  {
    // add movement in that direction
    AddMovementInput(GetActorForwardVector(), Value);
  }
}

void AMineCharacter::MoveRight(float Value)
{
  if (Value != 0.0f)
  {
    // add movement in that direction
    AddMovementInput(GetActorRightVector(), Value);
  }
}

void AMineCharacter::TurnAtRate(float Rate)
{
  // calculate delta for this frame from the rate information
  AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

/*void AMineCharacter::LookUpAtRate(float Rate)
{
  // calculate delta for this frame from the rate information
  AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}*/

void AMineCharacter::BreakBlockClient()
{
  UWorld* world = GetWorld();
  if (!world)
    return;

  APlayerCameraManager* camManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
  if (!camManager)
    return;

  FVector start = camManager->GetCameraLocation();
  FVector end = start + camManager->GetActorForwardVector() * 350.f;

  FCollisionQueryParams queryParam(FName("BlockCheck"), true, this);
  FHitResult hitResult;
  if (world->LineTraceSingleByChannel(hitResult, start, end, ECollisionChannel::ECC_Visibility, queryParam))
  {
    AChunk* chunk = Cast<AChunk>(hitResult.Actor);
    if (!chunk)
      return;

    FVector relativePos = (hitResult.Location + hitResult.Normal * -10.f) - chunk->GetActorLocation();

    FIntVector blockPos = FIntVector(
      FMath::FloorToInt(relativePos.X / (float)AChunk::CubeSize),
      FMath::FloorToInt(relativePos.Y / (float)AChunk::CubeSize),
      FMath::FloorToInt(relativePos.Z / (float)AChunk::CubeSize)
    );

    BreakBlock(start, chunk, blockPos);
  }
}

bool AMineCharacter::BreakBlock_Validate(FVector start, AChunk* chunk, FIntVector blockPos)
{
  return (chunk && blockPos.X < AChunk::CHUNKSIZEX && blockPos.Y < AChunk::CHUNKSIZEY && blockPos.Z < AChunk::CHUNKSIZEZ);
}

void AMineCharacter::BreakBlock_Implementation(FVector start, AChunk* chunk, FIntVector blockPos)
{
  if (!chunk)
    return;

  FBlock block;
  block.BlockType = 0u;

  chunk->SetBlock(blockPos, block);
  chunk->UpdateCompressedBlocks();
  chunk->FlushNetDormancy();
}

void AMineCharacter::PlaceBlockClient()
{
  UWorld* world = GetWorld();
  if (!world)
    return;

  APlayerCameraManager* camManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
  if (!camManager)
    return;

  FVector start = camManager->GetCameraLocation();
  FVector end = start + camManager->GetActorForwardVector() * 350.f;

  FCollisionQueryParams queryParam(FName("BlockCheck"), true, this);
  FHitResult hitResult;
  if (world->LineTraceSingleByChannel(hitResult, start, end, ECollisionChannel::ECC_Visibility, queryParam))
  {
    AChunk* chunk = Cast<AChunk>(hitResult.Actor);
    if (!chunk)
      return;

    FVector relativePos = (hitResult.Location + hitResult.Normal * 10.f) - chunk->GetActorLocation();

    FIntVector relativeBlockPos = FIntVector(
      FMath::FloorToInt(relativePos.X / (float)AChunk::CubeSize),
      FMath::FloorToInt(relativePos.Y / (float)AChunk::CubeSize),
      FMath::FloorToInt(relativePos.Z / (float)AChunk::CubeSize)
    );

    PlaceBlock(start, chunk, relativeBlockPos);
  }
}

bool AMineCharacter::PlaceBlock_Validate(FVector start, AChunk* chunk, FIntVector relativePos)
{
  return true;
}

void AMineCharacter::PlaceBlock_Implementation(FVector start, AChunk* chunk, FIntVector relativePos)
{
  if (!chunk)
    return;

  FBlock block;
  block.BlockType = 2u;

  //Outside of chunk bounds
  if (relativePos.X < 0 || relativePos.Y < 0 || relativePos.Z < 0 || relativePos.X >= AChunk::CHUNKSIZEX || relativePos.Y >= AChunk::CHUNKSIZEY || relativePos.Z >= AChunk::CHUNKSIZEZ)
  {
    FindChunkManager();

    if (!m_ChunkManager)
      return;

    FIntVector chunkPos = chunk->GetChunkPos();

    FIntVector newBlockSpacePos(
      chunkPos.X * AChunk::CHUNKSIZEX + relativePos.X,
      chunkPos.Y * AChunk::CHUNKSIZEY + relativePos.Y,
      chunkPos.Z * AChunk::CHUNKSIZEX + relativePos.Z
     );

    FIntVector newChunkPos(
      FMath::FloorToInt(newBlockSpacePos.X / (float)AChunk::CHUNKSIZEX),
      FMath::FloorToInt(newBlockSpacePos.Y / (float)AChunk::CHUNKSIZEY),
      FMath::FloorToInt(newBlockSpacePos.Z / (float)AChunk::CHUNKSIZEZ)
    );

    FIntVector newBlockPos(
      (newBlockSpacePos.X % AChunk::CHUNKSIZEX + AChunk::CHUNKSIZEX) % AChunk::CHUNKSIZEX,
      (newBlockSpacePos.Y % AChunk::CHUNKSIZEY + AChunk::CHUNKSIZEY) % AChunk::CHUNKSIZEY,
      (newBlockSpacePos.Z % AChunk::CHUNKSIZEZ + AChunk::CHUNKSIZEZ) % AChunk::CHUNKSIZEZ
    );

    AChunk* realChunk = m_ChunkManager->SetBlock(newChunkPos, newBlockPos, block);

    if (realChunk)
    {
      realChunk->UpdateCompressedBlocks();
      realChunk->FlushNetDormancy();
    }
  }
  //Inside of chunk bounds
  else
  {
    chunk->SetBlock(relativePos, block);
    chunk->UpdateCompressedBlocks();
    chunk->FlushNetDormancy();
  }
}

void AMineCharacter::FindChunkManager()
{
  if (!m_ChunkManager)
  {
    TArray<AActor*> arrayChunkManager;

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AChunkManager::StaticClass(), arrayChunkManager);
    check(arrayChunkManager.Num() > 0);

    m_ChunkManager = Cast<AChunkManager>(arrayChunkManager[0]);
    check(m_ChunkManager);
  }
}