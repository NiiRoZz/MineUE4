#include "Player.h"

namespace MineUE4Server
{
	Player::Player(ENetPeer* peer, std::size_t id)
	: m_Peer(peer)
	, m_ID(id)
	, m_EnetID(id)
	{
	}

	std::size_t Player::getID() const
	{
		return m_ID;
	}

	std::size_t& Player::getEnetID()
	{
		return m_EnetID;
	}
}