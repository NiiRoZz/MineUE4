#include "Game.h"

#include <iostream>

namespace MineUE4Server
{
	void Game::prePhysicsUpdate(double dt)
	{

	}

	void Game::update(double dt)
	{
		
	}

	void Game::postPhysicsUpdate(double dt)
	{
		
	}

	void Game::onNewPlayer(ENetPeer* playerPeer)
	{
		std::cout << "Game::OnNewPlayer 1 " << playerPeer << std::endl;
		if (playerPeer->data != nullptr)
			return;

		std::size_t playerID = m_Players.size();
		auto& addedPlayer = m_Players.emplace_back( std::make_unique<Player>(playerPeer, playerID) );

		playerPeer->data = new std::size_t;

		std::cout << "Game::OnNewPlayer 2 " << playerID << std::endl;
		*(std::size_t*)playerPeer->data = playerID;
	}

	void Game::onDisconnectedPlayer(ENetPeer* playerPeer)
	{
		if (playerPeer->data == nullptr)
			return;

		std::size_t playerIdx = *(std::size_t*)(playerPeer->data);

		std::cout << "Game::OnDisconnectedPlayer 1 " << playerIdx << std::endl;

		delete playerPeer->data;

		if (playerIdx >= m_Players.size())
			return;

		m_Players.erase(m_Players.begin() + playerIdx);
	}
}