// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkSubsystem.h"

#include "Engine/GameInstance.h"

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

	m_NetworkClient = enet_host_create(NULL /* create a client host */,
		1 /* only allow 1 outgoing connection */,
		1 /* allow up 1 channels to be used, 0 and 1 */,
		0 /* assume any amount of incoming bandwidth */,
		0 /* assume any amount of outgoing bandwidth */
	);

	if (!m_NetworkClient)
	{
		UE_LOG(LogTemp, Error, TEXT("UNetworkSubsystem::Initialize 1 m_NetworkClient is NULL"));
		return;
	}

	ENetAddress address;
	enet_address_set_host(&address, "127.0.0.1");
	address.port = 6502;

	m_NetworkPeer = enet_host_connect(m_NetworkClient, &address, 1, 0);
	
	if (!m_NetworkPeer)
	{
		UE_LOG(LogTemp, Error, TEXT("UNetworkSubsystem::Initialize 2 m_NetworkPeer is NULL"));
		return;
	}
}

void UNetworkSubsystem::Deinitialize()
{
	if (m_NetworkPeer)
	{
		enet_peer_disconnect(m_NetworkPeer, 0);

		/* Allow up to 3 seconds for the disconnect to succeed
		* and drop any packets received packets.
		*/
		ENetEvent event;
		while (enet_host_service(m_NetworkClient, &event, 3000) > 0)
		{
			switch (event.type)
			{
				case ENET_EVENT_TYPE_RECEIVE:
				{
					enet_packet_destroy(event.packet);
					break;
				}
				
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					UE_LOG(LogTemp, Warning, TEXT("UNetworkSubsystem::Deinitialize Disconnection succeeded."));
					enet_deinitialize();
				}
			}
		}
	}
}

void UNetworkSubsystem::Tick(float DeltaTime)
{
	if (!m_NetworkClient)
		return;

	ENetEvent event;

	//Dispatch all events
	while (enet_host_service(m_NetworkClient, &event, 0) > 0)
	{
		switch (event.type)
		{
			case ENET_EVENT_TYPE_CONNECT:
			{
				UE_LOG(LogTemp, Warning, TEXT("UNetworkSubsystem::Tick connect %x:%u"), event.peer->address.host, event.peer->address.port);

				/* Create a reliable packet of size 7 containing "packet\0" */
				ENetPacket* packet = enet_packet_create("packet",
					strlen("packet") + 1,
					ENET_PACKET_FLAG_RELIABLE);

				enet_peer_send(m_NetworkPeer, 0, packet);

				UE_LOG(LogTemp, Warning, TEXT("UNetworkSubsystem::Initialize sent packet."));

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
				UE_LOG(LogTemp, Warning, TEXT("UNetworkSubsystem::Tick disconnected."));

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
