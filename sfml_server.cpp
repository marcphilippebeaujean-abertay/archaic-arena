#include "sfml_server.h"

sfml_server::sfml_server(sf::IpAddress hostAddr, unsigned short tcpServerport, sf::Time blockingTimeval, int nrOfPlayers) : connection_manager(hostAddr, tcpServerport, blockingTimeval, true)
{
	// bind listener to port
	tcpListener = new sf::TcpListener;
	if (tcpListener->listen(tcpServerport) != sf::Socket::Done)
	{
		// failed to bind listener!
		std::cout << "Listener binding failed!" << std::endl;
	}
	// initialise sfml socket select
	selector = new sf::SocketSelector;
	// set listener for socket select
	selector->add(*tcpListener);
	// make sure that two players are connected to the server so they can play the game
	std::cout << "waiting for players to connect..." << std::endl;
	// define how many players connect before a match starts
	playerNr = nrOfPlayers;
}

sfml_server::~sfml_server()
{
	// close listener and disconnect client sockets
	tcpListener->close();
	for (auto i = 0; i < MAX_PLAYERS; i++)
	{
		connectedPlayers[i].clientSocket = nullptr;
	}
	
	if (tcpListener)
	{
		delete tcpListener;
		tcpListener = nullptr;
	}

	if (selector)
	{
		delete selector;
		selector = nullptr;
	}
}

void sfml_server::update(float& deltaTime)
{
	// all other players are connected, recieve and send data on the non-blocking sockets
	if (socketsConnected())
	{
		// recieve from the udp socket
		udpRecvMessage();
		// send udp if players are connected
		for (auto i = 0; i < getConnectionNr(); i++)
		{
			// make sure that the player is connected
			if (connectedPlayers[i].connected == true)
			{
				// send messages to each client
				unsigned short clientPort = 5000 + i;
				udpSendMessage(*getUdpSock(), clientPort);
			}
		}
		// the delta time on the first call after the game has been started can sometimes be very big, obscurring our calculations and disconnecting the socket
		if(deltaTime > 0.2f)
		{
			deltaTime = 0.2f;
		}
		incrementTimers(deltaTime);
		// read from clients for messages
		for (auto i = 0; i < getConnectionNr(); i++)
		{
			// make check before using tcp to see whether the client is connected
			// chek that we haven't already deleted this socket
				if(connectedPlayers[i].connected == true)
				{
					if(getTimer(i) > getPpTimeout())
					{
						disconnect(connectedPlayers[i].clientSocket);
						message disconnectMessage;
						disconnectMessage.initPacketValues(8, sf::Vector2f(0, 0), 0, i, 0.0f, message::MESSAGE_TYPE::TCP, 0.0f);
						connectedPlayers[i].connected = false;
						addToSendQueue(disconnectMessage);
						addToRcvQueue(disconnectMessage);
					}
					else
					{
						sf::TcpSocket& client = *connectedPlayers[i].clientSocket;
						// check if there are server messages to recieve
						tcpServerRecieveMessage(client);
					}
				}
		}
		// after reading for messages, send the messages to all clients
		for (auto i = 0; i < getConnectionNr(); i++)
		{
			// make sure that the player is connected
			if (connectedPlayers[i].connected == true)
			{
				sf::TcpSocket& client = *connectedPlayers[i].clientSocket;
				tcpSendMessage(client);
			}
		}
	}
	// use the selector to establish connections while we are in the lobby
	else
	{
		if (selector->wait(getBlockingTimeVal()))
		{
			// check if listener is ready to accept connections
			if (selector->isReady(*tcpListener))
			{
				// check that we have not gone over the maximum number of connections
				if (getConnectionNr() < MAX_PLAYERS && socketsConnected() == false)
				{
					// listener is ready: let's grab a new connection!
					sf::TcpSocket* client = new sf::TcpSocket;
					// check if the new client can be accepted
					if (tcpListener->accept(*client) == sf::Socket::Done)
					{
						// add new client to the selector
						selector->add(*client);
						// add new client to the connection list
						client->setBlocking(false);
						connectedPlayers[getConnectionNr()].clientSocket = client;
						// set the client to be connected
						connectedPlayers[getConnectionNr()].connected = true;
						// increment counter for the number of connections to the server
						incrementConnectionCounter();
						// create a new timer
						addTimer();
						// check if enough players have connected to start a game
						if (getConnectionNr() == playerNr)
						{
							// let's send a message back to all clients that the game is ready to start
							std::cout << getConnectionNr() << "players connected - ready to start the game!" << std::endl;
							handleGameStart();
						}
						else
						{
							std::cout << "need " << (playerNr - getConnectionNr()) << " more player(s) to connect" << std::endl;
						}
					}
					else
					{
						// otherwise, this is NOT a new client - let's get rid of it!
						delete client;
						client = nullptr;
					}
				}
			}
		}
	}
	clearSendQueue();
}


void sfml_server::tcpServerRecieveMessage(sf::TcpSocket& client)
{
	// recieve data that the client has sent
	sf::Packet packet;
	if (client.receive(packet) == sf::Socket::Done)
	{
		message msg;
		msg.unwrapPacket(packet);
		// add the message to our queue
		addToRcvQueue(msg);
		// since we want to send on this message to the next client, we need to add it to the to queue
		msg.setMsgType(message::MESSAGE_TYPE::TCP);
		addToSendQueue(msg);
	}
}


void sfml_server::handleGameStart()
{
	// generate udp socket
	initUdpSocket(0);
	// we don't want to accept more connections when we have started the game
	tcpListener->close();
	const sf::Int64 nrOfPlayers = getConnectionNr();
	// initialise players on the server before recieiving calls
	message initPlayersMessage;
	initPlayersMessage.initPacketValues(0, sf::Vector2f(0.0f, 0.0f), nrOfPlayers, 0, 0.0f, message::MESSAGE_TYPE::RECV, 0.0f);
	// add it to the queue
	addToRcvQueue(initPlayersMessage);
	// check each player's socket...
	for (auto i = 0; i < getConnectionNr(); i++)
	{
		// create a packet of information that we want to recieve from the client
		sf::Packet msgPacket;
		// generate variable that stores this player's ID
		const sf::Int16 playerID = i;
		// fill packet that we want to send
		msgPacket << playerID << nrOfPlayers;
		// attempt to send a message to this socket
		if (connectedPlayers[i].clientSocket->send(msgPacket) == sf::Socket::Done)
		{
			// sent message sucessfully
		}
		else
		{
			
		}
	}
	// confirm that the game has started
	verifyConnection();
}

void sfml_server::closeConnection()
{
	// clear selector
	selector->clear();
	// reset listener
	tcpListener->close();
	// delete all client connections
	for(auto i = 0; i < MAX_PLAYERS; i++)
	{
		connectedPlayers[i].clientSocket->disconnect();
		delete connectedPlayers[i].clientSocket;
		connectedPlayers[i].connected = false;
	}
	// close socket info
	resetConnectionStatus();
}
