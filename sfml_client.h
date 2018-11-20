#include "connection_manager.h"
#pragma once
class sfml_client : public connection_manager
{
public:
	sfml_client(sf::IpAddress hostAddr, unsigned short serverport, sf::Time blockingTimeval);
	~sfml_client();

	// override functions
	void update(float& deltaTime) override;
	void handleGameStart() override;
	void closeConnection() override;
	
	void tcpRecieveMessage();

	// getters
	int getPlID() { return playerID; }
private:
	// connects the client to the server
	void connectTcpSocket();
	// socket used for tcp data transfer
	sf::TcpSocket* tcpSock;
	// check for if the player connected
	bool isConnected;
	// indicates the ID of the player
	sf::Int16 playerID;
	bool connectionClosed = false;
};


