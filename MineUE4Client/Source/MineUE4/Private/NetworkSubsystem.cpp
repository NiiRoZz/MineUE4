// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkSubsystem.h"

#include "Engine/GameInstance.h"

#include "Windows/PreWindowsApi.h"
#include "reactphysics3d/reactphysics3d.h"
#include "Windows/PostWindowsApi.h"

UNetworkSubsystem* UNetworkSubsystem::GetNetworkInstance()
{
	if (GEngine->GetWorld() != nullptr && GEngine->GetWorld()->GetGameInstance() != nullptr)
	{
		if (UGameInstance* gameInstance = GEngine->GetWorld()->GetGameInstance())
		{
			return gameInstance->GetSubsystem<UNetworkSubsystem>();
		}
	}

	return nullptr;
}

void UNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	enet_initialize();

	UE_LOG(LogTemp, Warning, TEXT("UNetworkSubsystem::Initialize 1"));

	#if UE_SERVER
	UE_LOG(LogTemp, Warning, TEXT("UNetworkSubsystem::Initialize 2"));
	#endif

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = 1234;

	m_Host = enet_host_create(&address, 100, 2, 0, 0);
}

void UNetworkSubsystem::Deinitialize()
{
	enet_deinitialize();

	UE_LOG(LogTemp, Warning, TEXT("UNetworkSubsystem::Deinitialize"));
}

void UNetworkSubsystem::Tick(float DeltaTime)
{
	ENetEvent event;

	//Dispatch all events
	while (enet_host_service(m_Host, &event, 0) > 0)
	{
		switch (event.type)
		{
			case ENET_EVENT_TYPE_CONNECT:
			{
				UE_LOG(LogTemp, Warning, TEXT("UNetworkSubsystem::Tick connect %x:%u"), event.peer->address.host, event.peer->address.port);
				/* Store any relevant client information here. */
				event.peer->data = "Client information";

				break;
			}
				
			case ENET_EVENT_TYPE_RECEIVE:
			{
				UE_LOG(LogTemp, Warning, TEXT("UNetworkSubsystem::Tick A packet of length %u containing %s was received from %s on channel %u."), event.packet->dataLength, event.packet->data, event.peer->data, event.channelID);
				/* Clean up the packet now that we're done using it. */
				enet_packet_destroy(event.packet);

				break;
			}
				

			case ENET_EVENT_TYPE_DISCONNECT:
			{
				UE_LOG(LogTemp, Warning, TEXT("UNetworkSubsystem::Tick %s disconnected."), event.peer->data);
				/* Reset the peer's client information. */
				event.peer->data = NULL;

				break;
			}

			default:
			{
				
			}
		}
	}
}

bool UNetworkSubsystem::IsTickableInEditor() const
{
	return false;
}

TStatId UNetworkSubsystem::GetStatId() const
{
	return TStatId();
}

ETickableTickType UNetworkSubsystem::GetTickableTickType() const
{
	return ETickableTickType::Always;
}
