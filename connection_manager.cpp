#include "connection_manager.h"

connection_manager::connection_manager(sf::IpAddress hostAddr, unsigned short tcpServerport, sf::Time blockingTimeVal, bool isServer)
{
	// define address and port we want to connect to
	connectAddr = hostAddr;
	this->tcpServerport = tcpServerport;
	// set the amount of time that the socket blocks for each select call
	this->blockingTimeVal = blockingTimeVal;
	// initialise sockets
	udpSock = new sf::UdpSocket;
	udpSock->setBlocking(false);
	// initialise game loop dependancies
	playersConnected = false;
	// determine if the manager is the server or not
	this->server = isServer;
	// initialise connection counter
	connectionCounter = 0;
	ppTimeout = 3.0f;
}

connection_manager::~connection_manager()
{
	if (udpSock)
	{
		delete udpSock;
		udpSock = nullptr;
	}
	// clear message vector
	queuedRecievedMessages.clear();
	queuedOutgoingMessages.clear();
}

void connection_manager::update(float& deltaTime)
{

}

void connection_manager::handleGameStart()
{

}

void connection_manager::addToRcvQueue(message msg)
{
	// store the protocol message
	queuedRecievedMessages.push_back(msg);
}

void connection_manager::tcpSendMessage(sf::TcpSocket& tcpSock)
{
	// iterate through each message in the queue
	for (auto i = 0; i < queuedOutgoingMessages.size(); i++)
	{
		// check that this message was supposed to be sent with TCP
		if(queuedOutgoingMessages[i].getMessageType() == message::MESSAGE_TYPE::TCP)
		{
			// if we find a message, convert it into a packet
			sf::Packet msgPacket = getOutgoingMessageQueue()[i].createPacket();
			switch (tcpSock.send(msgPacket))
			{
			case sf::Socket::Done:
				{
				// send the message
				if (server == false)
					{
					// erase the message from the queue - we can't do that for the server because we need to send each message out to all clients
					queuedOutgoingMessages.erase(queuedOutgoingMessages.begin() + i);
					}
				break;
				}
			case sf::Socket::NotReady:
				{
					// No more messages to receive.
					return;
				}
			default:
				{
					// something happened
					return;
				}
			}
		}
	}
}

void connection_manager::initUdpSocket(int id)
{
	// modify udp port to bind to a unique port
	if(server == false)
	{
		udpPort = 5000 + id;
	}
	else
	{
		udpPort = udpServerPort;
	}
	// bind it to the server's address and the port
	udpSock->bind(udpPort);
}

void connection_manager::udpRecvMessage()
{
	// create message variable to store packet information
	message recvMsg;
	sf::Packet packet;
	// get sender ip address and port
	sf::IpAddress sender;
	unsigned short port;

	while (true) 
	{	
		switch (udpSock->receive(packet, sender, port))
		{
		case sf::Socket::Done:
			// if we have recieved a packet, unwrap it...
			recvMsg.unwrapPacket(packet);
			// ...and push it to the queue
			queuedRecievedMessages.push_back(recvMsg);
			// check if we are the server
			if (server)
			{
				// make sure we send this message via UDP
				recvMsg.setMsgType(message::MESSAGE_TYPE::UDP);
				// we want to add the message to our outgoing so all clients can recieve it
				queuedOutgoingMessages.push_back(recvMsg);
			}
			break;
		case sf::Socket::NotReady:
			// No more messages to receive.
			return;
		default:
			// Error has occurred -- handle it.
			// FIXME
			break;
		}
	}
}

void connection_manager::udpSendMessage(sf::UdpSocket& socket, const unsigned short targetPort)
{
	// iterate through the message queue
	for(auto i = 0; i < queuedOutgoingMessages.size(); i++)
	{
		// check if the message needs to be sent via udp
		if(queuedOutgoingMessages[i].getMessageType() == message::MESSAGE_TYPE::UDP)
		{
			// create the packet based on the queued message
			sf::Packet sendPacket = queuedOutgoingMessages[i].createPacket();
			// send the packet to the target host
			if(socket.send(sendPacket, connectAddr, targetPort) == sf::Socket::Done)
			{
				// send the message
				if (server == false)
				{
					// erase the message from the queue - we can't do that for the server because we need to send each message out to all clients
					queuedOutgoingMessages.erase(queuedOutgoingMessages.begin() + i);
				}
			}
		}
	}
}

void connection_manager::closeConnection()
{
	
}

void connection_manager::incrementTimers(float deltaTime)
{
	for(auto i = 0; i < ppTimers.size(); i++)
	{
		if(ppTimers[i] < ppTimeout)
		{
			// iterate through each timer and increment it
			ppTimers[i] += deltaTime;
		}
	}
}

void connection_manager::disconnect(sf::TcpSocket* socket)
{

	// disconnect the socket
	socket->disconnect();
	delete socket;
	socket = nullptr;
}
