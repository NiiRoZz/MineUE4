#pragma once

#include "enet/enet.h"

#include <cstddef>

namespace MineUE4Server
{
	class Player
	{
		public:
			Player(ENetPeer* peer, std::size_t id);

			std::size_t getID() const;

			//Should only be used by enet
			std::size_t& getEnetID();

		private:
			ENetPeer* m_Peer;
			const std::size_t m_ID;
			std::size_t m_EnetID;
	};
}