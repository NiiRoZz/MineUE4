// Copyright Epic Games, Inc. All Rights Reserved.


#include "MineUE4GameModeBase.h"

AMineUE4GameModeBase::AMineUE4GameModeBase()
  : Super()
{
  // set default pawn class to our Blueprinted character
  static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/BP_MineCharacter"));
  DefaultPawnClass = PlayerPawnClassFinder.Class;
}
