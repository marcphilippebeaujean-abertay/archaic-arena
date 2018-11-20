#include "sfml_client.h"

sfml_client::sfml_client(sf::IpAddress hostAddr, unsigned short tcpServerport, sf::Time blockingTimeval) : connection_manager(hostAddr, tcpServerport, blockingTimeval, false)
{
	// initialise tcp socket
	tcpSock = new sf::TcpSocket;
	// set tcp socket to unblocking when the game loads
	tcpSock->setBlocking(false);
	// initialise connection to false
	isConnected = false;
}

sfml_client::~sfml_client()
{
	if (tcpSock)
	{
		if (!tcpSock->Disconnected)
		{
			// gracefully disconnect socket before removing pointer
			tcpSock->disconnect();
		}
		delete tcpSock;
		tcpSock = nullptr;
	}
}

void sfml_client::update(float& deltaTime)
{
	// check if client has realised that the game has started
	if (!isConnected)
	{
		connectTcpSocket();
	}
	else
	{
		if (socketsConnected() == false)
		{
			// wait for server confirmation that the game can start
			handleGameStart();
		}
		else
		{
			// the delta time on the first call after the game has been started can sometimes be very big, obscurring our calculations and disconnecting the socket
			if(deltaTime > 0.1)
			{
				deltaTime = 0.1;
			}
			incrementTimers(deltaTime);
			// check if the last message we have recieved has been a certain time ago
			if (getTimer(0) > getPpTimeout())
			{
				message disconnectMessage;
				disconnectMessage.initPacketValues(9, sf::Vector2f(0, 0), 0, 0, 0.0f, message::MESSAGE_TYPE::TCP, 0.0f);
				addToRcvQueue(disconnectMessage);
				closeConnection();
			}
			else
			{
				// send and recieve messages
				tcpSendMessage(*tcpSock);
				tcpRecieveMessage();
				udpSendMessage(*getUdpSock(), getUdpServerPort());
				udpRecvMessage();
			}
		}
	}
	// clear the queue
	getOutgoingMessageQueue().clear();
}

void sfml_client::handleGameStart()
{
	// indicate that we have established and not closed a connection
	connectionClosed = false;
	// create a packet we want to recieve
	sf::Packet gameStartPacket;
	// attempt to recieve the message
	if (tcpSock->receive(gameStartPacket) == sf::Socket::Done)
	{
		// create a variable to store the number of players recieved from the server
		sf::Int64 nrOfPlayers;
		// attempt to store packet information
		if (gameStartPacket >> playerID >> nrOfPlayers)
		{
			// we want to initialise our networked player using the game manager queue - given the nature of this first call, we need to do it manually
			message initMsg;
			initMsg.initPacketValues(0, sf::Vector2f(0, 0), nrOfPlayers, playerID, 0.0f, message::MESSAGE_TYPE::TCP, 0.0f);
			// add it to the queue
			addToRcvQueue(initMsg);
			// setup udp socket
			initUdpSocket(playerID);
			// create a ping-pong timer for the client
			addTimer();
			// start the game
			verifyConnection();
		}
		else
		{
			std::cout << "failed to encrpyt packet!" << std::endl;
		}
	}
	else
	{
		// failed to recieve packet
	}
}

void sfml_client::tcpRecieveMessage()
{
	sf::Packet receptionPacket;
	// attempt to recieve a message
	if (tcpSock->receive(receptionPacket) == sf::Socket::Done)
	{
		// we have recieved a packet - let's translate it into a message
		message recvMessage;
		recvMessage.unwrapPacket(receptionPacket);
		// then, add the message to the queue of recieved messages
		addToRcvQueue(recvMessage);
	}
}

void sfml_client::connectTcpSocket()
{
	// create a tcp connection
	sf::Socket::Status netState;
	netState = tcpSock->connect(getConnectAddr(), getTcpServerPort());
	// determine if connection was successful
	if (netState != sf::Socket::Done)
	{
		// we failed to connect!
	}
	else
	{	
		// recieve the start notification
		isConnected = true;
		incrementConnectionCounter();
	}
}

void sfml_client::closeConnection()
{
	if(connectionClosed == false)
	{
		// close sockets gracefully
		tcpSock->disconnect();
		// reset variable used to send information
		resetConnectionStatus();
		isConnected = false;
		connectionClosed = true;
	}
}
