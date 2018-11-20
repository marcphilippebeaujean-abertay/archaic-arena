#pragma once
#include "connection_manager.h"

class sfml_server : public connection_manager
{
public:
	sfml_server(sf::IpAddress hostAddr, unsigned short serverport, sf::Time blockingTimeval, int nrOfPlayers);
	~sfml_server();

	// override functions
	void update(float& deltaTime) override;
	void handleGameStart() override;
	void closeConnection() override;

	// getters

	void tcpServerRecieveMessage(sf::TcpSocket& client);

private:
	struct Connection
	{
		sf::TcpSocket* clientSocket;
		bool connected;
	};
	// define the nr of players that need to connect before a game starts
	int playerNr = 1;
	// define the maximum nr of players for a game
	#define MAX_PLAYERS 4
	// listener used to grab TCP connections
	sf::TcpListener* tcpListener;
	// vector that stores information on the connections made
	Connection connectedPlayers[MAX_PLAYERS];
	// selector that is used to wait and handle multiple connections
	sf::SocketSelector* selector;
};

