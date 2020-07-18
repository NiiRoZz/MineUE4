#include <iostream>
#include <reactphysics3d/reactphysics3d.h>
#include <enet/enet.h>

int main()
{
    std::cout << "Hello World! 1" << std::endl;

		rp3d::PhysicsCommon physicsCommon;

		rp3d::PhysicsWorld* world = physicsCommon.createPhysicsWorld();

		enet_initialize();

		atexit(enet_deinitialize);

		std::cout << "Hello World! 2" << std::endl;
}