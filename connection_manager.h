#pragma once
#include <iostream>
#include <SFML\Network.hpp>
#include "message.h"

class connection_manager
{
public:
	connection_manager(sf::IpAddress hostAddr, unsigned short serverport, sf::Time blockingTimeVal, bool isServer);
	~connection_manager();

	// getters
	sf::UdpSocket* getUdpSock() { return udpSock; }
	sf::IpAddress& getConnectAddr() { return connectAddr; }
	sf::Time& getBlockingTimeVal() { return blockingTimeVal; }
	std::vector<message> getRecievedMessageQueue() { return queuedRecievedMessages; }
	std::vector<message> getOutgoingMessageQueue() { return queuedOutgoingMessages; }
	unsigned short& getTcpServerPort() { return tcpServerport; }
	unsigned short& getUdpPort() { return udpPort; }
	unsigned short& getUdpServerPort() { return udpServerPort; }
	bool& socketsConnected() { return playersConnected; }
	bool& isServer() { return server; }
	int getMaxPlayers() { return maxPlayers; }
	float& getPpTimeout() { return ppTimeout; }
	float getTimer(int id) { return ppTimers[id]; }
	int& getConnectionNr() { return connectionCounter; }
	// setters
	void verifyConnection() { playersConnected = true; }
	void resetConnectionStatus() { playersConnected = false; }
	void addTimer() { ppTimers.push_back(0.0f); }
	void incrementTimers(float deltaTime);
	void resetTimer(int id) { ppTimers[id] = 0.0f; }
	void incrementConnectionCounter() { connectionCounter++; }

	// virtual
	virtual void update(float& deltaTime);
	virtual void handleGameStart();
	virtual void closeConnection();

	// protocol sending / recieving
	void addToRcvQueue(message packet);
	void clearRcvQueue() { queuedRecievedMessages.clear(); }
	void clearSendQueue() { queuedOutgoingMessages.clear(); }
	void addToSendQueue(message msg) { queuedOutgoingMessages.push_back(msg); }
	void tcpSendMessage(sf::TcpSocket& tcpSock);
	void udpSendMessage(sf::UdpSocket& socket, const unsigned short targetPort);
	void udpRecvMessage();
	void initUdpSocket(int id);
	void disconnect(sf::TcpSocket* socket);

private:
	// define the port we will be using to establish a base TCP connection between server and clients (along with other important cals)
	unsigned short tcpServerport;
	// server port assigned to the udp socket - needs to be different for each client and server
	unsigned short udpPort;
	// port that is used exclusively by the server and the address for client messages
	unsigned short udpServerPort = 5500;
	// we will only need one UDP socket regardless of whether we are client or server
	sf::UdpSocket* udpSock;
	// the address we want to connect to
	sf::IpAddress connectAddr;
	// time that select blocks
	sf::Time blockingTimeVal;
	// returns true if the network manager acts as the game's server
	bool server;
	// bool that checks if all sockets are connected and the game is ready to commence
	bool playersConnected;
	// vector holding the messages recieved from sockets
	std::vector<message> queuedRecievedMessages;
	// vector holding the messages we need to send
	std::vector<message> queuedOutgoingMessages;
	// maximum number of players that can join a game
	const int maxPlayers = 4;
	// create timers for "ping-ponging"
	std::vector<float> ppTimers;
	// check for how long before the socket counts as disconnected
	float ppTimeout;
	// increment that determines how many connections the server has made - helps populate connection array
	int connectionCounter;
};

