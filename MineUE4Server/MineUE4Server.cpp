#include <iostream>

#define IS_RP3D_DOUBLE_PRECISION_ENABLED
#include "reactphysics3d/reactphysics3d.h"

#include "enet/enet.h"

int main()
{
		enet_initialize();
		atexit(enet_deinitialize);

		ENetAddress address;
		address.host = ENET_HOST_ANY;
		address.port = 6502;

		ENetHost* server = enet_host_create( &address /* the address to bind the server host to */,
			100			/* allow up to 100 clients and/or outgoing connections */,
			1				/* allow up to 1 channels to be used, 0 only */,
			0				/* assume any amount of incoming bandwidth */,
			0				/* assume any amount of outgoing bandwidth */
		);

		if (server == nullptr)
		{
			fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
			exit(EXIT_FAILURE);
		}

		// First you need to create the PhysicsCommon object .
		// This is a factory module that you can use to create physics
		// world and other objects . It is also responsible for
		// logging and memory management
		rp3d::PhysicsCommon physicsCommon;
		// Create a physics world
		rp3d::PhysicsWorld* world = physicsCommon.createPhysicsWorld();
		// Create a rigid body in the world
		rp3d::Vector3 position(0, 20, 0);
		rp3d::Quaternion orientation = rp3d::Quaternion::identity();
		rp3d::Transform transform(position, orientation);
		rp3d::RigidBody* body = world->createRigidBody(transform);

		const rp3d::decimal timeStep = 1.0f / 60.0f;

		auto previousFrameTime = std::chrono::high_resolution_clock::now();
		double accumulator = 0;

		bool running = true;

		//Server loop
		while (running)
		{
			auto currentFrameTime = std::chrono::high_resolution_clock::now();

			double deltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentFrameTime - previousFrameTime).count();
			deltaTime *= 1e-9;

			// Update the previous time
			previousFrameTime = currentFrameTime;

			// Add the time difference in the accumulator
			accumulator += deltaTime;

			//Dispatch network events
			ENetEvent event;
			while (enet_host_service(server, &event, 0) > 0)
			{
				switch (event.type)
				{
					case ENET_EVENT_TYPE_CONNECT:
					{
						printf("A new client connected from %x:%u.\n",
							event.peer->address.host,
							event.peer->address.port);
						/* Store any relevant client information here. */
						//event.peer->data = "Client information";

						break;
					}
					
					case ENET_EVENT_TYPE_RECEIVE:
					{
						printf("A packet of length %u containing %s was received on channel %u.\n",
							event.packet->dataLength,
							event.packet->data,
							event.channelID);
						/* Clean up the packet now that we're done using it. */
						enet_packet_destroy(event.packet);

						break;
					}
					

					case ENET_EVENT_TYPE_DISCONNECT:
					{
						printf("%s disconnected.\n", event.peer->data);
						/* Reset the peer's client information. */
						event.peer->data = NULL;

						break;
					}
				}
			}


			//Update physics

			// While there is enough accumulated time to take
			// one or several physics steps
			while (accumulator >= timeStep)
			{
				// Update the Dynamics world with a constant time step
				world->update(timeStep);
				// Decrease the accumulated time
				accumulator -= timeStep;
			}

			
			/*
			// Get the updated position of the body
			const rp3d::Transform& transform = body->getTransform();
			const rp3d::Vector3& position = transform.getPosition();
			// Display the position of the body
			std::cout << " Body Position : (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
			*/
		}

		enet_host_destroy(server);

		return 0;
}