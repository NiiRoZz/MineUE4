#pragma once

#include "Player.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace MineUE4Server
{
	class Game
	{
		public:

			//Called each frame
			void update(double dt);

			//Called each time before physic update, could be called multiple times each frame if needed
			void prePhysicsUpdate(double dt);

			//Called each time after physic update, could be called multiple times each frame if needed
			void postPhysicsUpdate(double dt);

			//Called each time new player connect
			void onNewPlayer(ENetPeer* playerPeer);

			//Called each time a player disconnect
			void onDisconnectedPlayer(ENetPeer* playerPeer);

		private:
			
			std::vector< std::unique_ptr<Player> > m_Players;
	};
}